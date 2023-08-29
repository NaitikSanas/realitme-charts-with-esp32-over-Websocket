#include "main.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#define TAG "dashboard"
#include "websocket_server_api/websocket_server.h"

extern const uint8_t chart_js_start[]  asm("_binary_chart_js_start");
extern const uint8_t chart_js_end[]	   asm("_binary_chart_js_end");

extern const uint8_t index_js_start[]  asm("_binary_index_js_start");
extern const uint8_t index_js_end[]	   asm("_binary_index_js_end");

extern const uint8_t index_html_start[]  asm("_binary_index_html_start");
extern const uint8_t index_html_end[]	   asm("_binary_index_html_end");

static esp_err_t http_server_chart_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "chart.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)chart_js_start, chart_js_end - chart_js_start);

	return ESP_OK;
}


static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}


static esp_err_t http_server_index_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)index_js_start, index_js_end - index_js_start);

	return ESP_OK;
}



void add_new_uri_handler(httpd_handle_t server_handle, char* uri,httpd_method_t method, esp_err_t (*handler)(httpd_req_t *r), void *user_ctx){
    httpd_uri_t uri_handler = {
				.uri = uri,
				.method = method,
				.handler = handler,
				.user_ctx = user_ctx
    };
    httpd_register_uri_handler(server_handle, &uri_handler);
}

void start_server (void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t http_server_handle = NULL;
    printf("STARTING SERVER>>>>\r\n");
    config.stack_size = 4096;
	config.task_priority = 2;
	config.max_uri_handlers = 12;
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{  
        ws_server_start(http_server_handle);
        add_new_uri_handler(
            http_server_handle, 
            "/chart.js",
            HTTP_GET, 
            http_server_chart_js_handler, 
            NULL
        );

        add_new_uri_handler(
            http_server_handle,
            "/",
            HTTP_GET,
            http_server_index_html_handler,
            NULL
        );

        add_new_uri_handler(
            http_server_handle,
            "/index.js",
            HTTP_GET,
            http_server_index_js_handler,
            NULL
        );
    }
}