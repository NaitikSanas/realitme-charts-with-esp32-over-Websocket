#include "string.h"
#include "esp_http_server.h"
#include "esp_log.h"

#define WS_DATA_FIELD_SIZE 1024
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

void ws_server_start(httpd_handle_t http_server_handle);
int ws_set_data_fields(uint8_t* data, uint16_t len, bool clear_on_send);
int ws_data_received_cb(uint8_t* data, size_t len);