#include "websocket_server.h"
#define TAG "WS_Server"
#include "freertos/semphr.h" // Include the semaphore header

bool clear_on_send = false;
char ws_data_field_last[WS_DATA_FIELD_SIZE]={0};
char ws_data_field[WS_DATA_FIELD_SIZE]={0};
SemaphoreHandle_t ws_df_semaphore;
int ws_set_data_fields(uint8_t* data, uint16_t len, bool clear_on_send){
    if (xSemaphoreTake(ws_df_semaphore, portMAX_DELAY) == pdTRUE){
        if(len > WS_DATA_FIELD_SIZE){
            return -1;
        }   
        memset(ws_data_field,0,WS_DATA_FIELD_SIZE);
        memcpy(ws_data_field,data,len);
        xSemaphoreGive(ws_df_semaphore);
        clear_on_send = clear_on_send;
    }
    return 0;
}

__weak_symbol int ws_data_received_cb(uint8_t* data, size_t len){
    printf("[WS_CB] received data %s, len %d\r\n",(char*)data,len);
    return 0;
}

static void ws_async_send(void *arg)
{
    if (xSemaphoreTake(ws_df_semaphore, portMAX_DELAY) == pdTRUE){
        struct async_resp_arg *resp_arg = arg;
        httpd_handle_t hd = resp_arg->hd;
        int fd = resp_arg->fd;
        httpd_ws_frame_t ws_pkt;
        
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t*)ws_data_field;
        ws_pkt.len = strlen(ws_data_field);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        httpd_ws_send_frame_async(hd, fd, &ws_pkt);
        free(resp_arg);
        // if(clear_on_send)memset(ws_data_field,0,WS_DATA_FIELD_SIZE);
    xSemaphoreGive(ws_df_semaphore);
    }
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}


static esp_err_t handle_ws_req(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        
    }
    //trigger_async_send(req->handle, req);
   // ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char *)ws_pkt.payload, "data_request") == 0)
    {
        
        free(buf);
        return trigger_async_send(req->handle, req);
    }
    else {
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    return ESP_OK;
}
void ws_server_start(httpd_handle_t http_server_handle){
    ESP_LOGI(TAG,"Websocket server started");
    ws_df_semaphore = xSemaphoreCreateBinary(); // Create the binary semaphore
    xSemaphoreGive(ws_df_semaphore); // Give the semaphore initially
    static httpd_uri_t ws = {
				.uri        = "/ws",
				.method     = HTTP_GET,
				.handler    = handle_ws_req,
				.user_ctx   = NULL,
				.is_websocket = true
		};	
    httpd_register_uri_handler(http_server_handle, &ws);
}