#ifndef _WEBSOCKET_CONTROL_SERVER_H_
#define _WEBSOCKET_CONTROL_SERVER_H_

#include <esp_http_server.h>

class WebSocketControlServer {
public:
    WebSocketControlServer();
    ~WebSocketControlServer();

    // 统一使用 int 类型，避免签名不匹配
    bool Start(int port = 8080);
    void SendMessage(int fd, const char *message);

private:
    httpd_handle_t server_;
    static esp_err_t WebSocketHandler(httpd_req_t *req);
};

#endif // _WEBSOCKET_CONTROL_SERVER_H_
