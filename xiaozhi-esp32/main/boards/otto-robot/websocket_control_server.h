#ifndef _WEBSOCKET_CONTROL_SERVER_H_
#define _WEBSOCKET_CONTROL_SERVER_H_

#include <esp_http_server.h>

class WebSocketControlServer {
public:
    WebSocketControlServer();
    ~WebSocketControlServer();

    bool Start(int port = 8080);
    void SendMessage(int fd, const char *message);

private:
    httpd_handle_t server_;
    static esp_err_t WebSocketHandler(httpd_req_t *req);
};

#endif // _WEBSOCKET_CONTROL_SERVER_H_
