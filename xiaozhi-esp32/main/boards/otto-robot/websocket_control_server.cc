#include "websocket_control_server.h"

#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>

#define TAG "WebSocketControl"

// 异步发送任务的数据结构
struct async_send_job {
    httpd_handle_t server;
    int fd;
    char *data;
    size_t len;
};

// 异步发送任务
static void async_send_task(void *arg) {
    async_send_job *job = (async_send_job *)arg;
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)job->data;
    ws_pkt.len = job->len;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // 发送 WebSocket 帧
    esp_err_t ret = httpd_ws_send_frame_async(job->server, job->fd, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send frame: %d", ret);
    }
    
    // 清理内存
    free(job->data);
    free(job);
    vTaskDelete(NULL);
}

WebSocketControlServer::WebSocketControlServer() : server_(nullptr) {}

WebSocketControlServer::~WebSocketControlServer() {
    if (server_) {
        httpd_stop(server_);
    }
}

// WebSocket 接收消息的处理函数
esp_err_t WebSocketControlServer::WebSocketHandler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, new connection opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // 获取帧长度
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "recv_frame failed to get frame len: %d", ret);
        return ret;
    }

    if (ws_pkt.len) {
        buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "recv_frame failed: %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got message: %s", buf);
    }
    
    free(buf);
    return ESP_OK;
}

// 发送消息的具体实现
void WebSocketControlServer::SendMessage(int fd, const char *message) {
    if (!server_ || !message) return;
    
    async_send_job *job = (async_send_job *)malloc(sizeof(async_send_job));
    if (!job) return;
    
    job->server = server_;
    job->fd = fd;
    job->len = strlen(message);
    job->data = strdup(message);
    
    if (!job->data) {
        free(job);
        return;
    }
    
    // 创建任务进行异步发送，避免阻塞当前流程
    xTaskCreate(async_send_task, "ws_send_task", 4096, job, 5, NULL);
}

// 启动服务器
bool WebSocketControlServer::Start(int port) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = port + 1; 
    config.max_open_sockets = 7;

    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSocketHandler,
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
