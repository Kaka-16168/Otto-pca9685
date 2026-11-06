#include "websocket_control_server.h"

#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// 注意：如果你的工程需要控制 Otto，请保留原有的 Otto 头文件引用
// #include "otto_controller.h" 

#define TAG "WebSocketControl"

WebSocketControlServer::WebSocketControlServer() : server_(nullptr) {}

WebSocketControlServer::~WebSocketControlServer() {
    if (server_) {
        httpd_stop(server_);
    }
}

// WebSocket 接收消息的处理函数
esp_err_t WebSocketControlServer::WebSocketHandler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // 获取帧长度
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len) {
        buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got message: %s", buf);
        
        // 在这里处理你的控制逻辑，例如：
        // ParseCommand((char*)buf);
    }
    
    free(buf);
    return ESP_OK;
}

// 异步发送任务的参数结构
struct async_send_job {
    httpd_handle_t server;
    int fd;
    char *data;
    size_t len;
};

// 异步发送任务
static void async_send_task(void *arg) {
    async_send_job *job = (async_send_job *)arg;
    
    // 修复点：正确定义 httpd_ws_frame_t 并初始化
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)job->data;
    ws_pkt.len = job->len;
    // 修复点：使用正确的枚举值 HTTPD_WS_TYPE_TEXT
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // 修复点：确保包含 <esp_http_server.h> 后调用此函数
    esp_err_t ret = httpd_ws_send_frame_async(job->server, job->fd, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send frame: %d", ret);
    }
    
    free(job->data);
    free(job);
    vTaskDelete(NULL);
}

// 发送消息接口
void WebSocketControlServer::SendMessage(int fd, const char *message) {
    if (!server_) return;
    
    async_send_job *job = (async_send_job *)malloc(sizeof(async_send_job));
    if (!job) return;
    
    job->server = server_;
    job->fd = fd;
    job->len = strlen(message);
    job->data = strdup(message);
    
    xTaskCreate(async_send_task, "ws_send_task", 4096, job, 5, NULL);
}

bool WebSocketControlServer::Start(uint16_t port) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = port + 1; 
    config.max_open_sockets = 7;

    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = [](httpd_req_t *req) -> esp_err_t {
            // C 语言回调需要静态函数或无捕获的 lambda
            if (req->method == HTTP_GET) {
                return ESP_OK;
            }
            // 简化的接收逻辑
            httpd_ws_frame_t ws_pkt;
            memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
            ws_pkt.type = HTTPD_WS_TYPE_TEXT;
            esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
            if (ret == ESP_OK && ws_pkt.len) {
                uint8_t *buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
                ws_pkt.payload = buf;
                httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
                ESP_LOGI(TAG, "RX: %s", buf);
                free(buf);
            }
            return ESP_OK;
        },
        .user_ctx = NULL,
        .is_websocket = true
    };

    if (httpd_start(&server_, &config) == ESP_OK) {
        httpd_register_uri_handler(server_, &ws_uri);
        ESP_LOGI(TAG, "WebSocket server started on port %d", port);
        return true;
    }
    return false;
}
