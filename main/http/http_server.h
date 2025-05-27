#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "esp_err.h"

// HTTP Server management
esp_err_t http_server_init(void);
esp_err_t http_server_start(void);
esp_err_t http_server_stop(void);
bool http_server_is_running(void);

// Server configuration
void http_server_set_port(uint16_t port);
void http_server_set_stack_size(size_t stack_size);
void http_server_set_max_connections(int max_connections);

// Route registration helpers
esp_err_t http_server_register_light_routes(void);
esp_err_t http_server_register_ota_routes(void);
esp_err_t http_server_register_all_routes(void);

#endif // HTTP_SERVER_H