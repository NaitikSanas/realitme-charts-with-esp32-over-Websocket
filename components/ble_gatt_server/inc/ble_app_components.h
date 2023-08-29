
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include <string.h>
#include "sdkconfig.h"
#include "freertos/ringbuf.h"
//#include "flash_rw.h"
#define GATTS_TAG "GATTS_DEMO"
typedef struct 
{
    char* data;
    uint16_t size;
}ble_payload;

typedef struct
{
esp_gatt_if_t gatts_if;
uint16_t conn_id;
uint16_t char_handle;
}BLE_ctx;

#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

RingbufHandle_t buf_handle;
typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

prepare_type_env_t a_prepare_write_env;
prepare_type_env_t b_prepare_write_env;

/*
    This Funciton Handles GAP related Events It Start Advertisment,
     Establishes connection.
*/
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param); //GAP Event Handler

/*
    This Funciton Sends back response to BLE Client when the Write Request from Client comes
*/
void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param); 

/*
    This Funciton Sends back response to BLE Client when the Write Request from Client comes
*/
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

/*
    This Is the main Server Profile Handler. 
    All the Read/Write Connect/disconnect event are handled here
*/
void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param); 

/*
    This Funciton Handles GATT Server related events.
*/
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*
    This funtion returns BLE Connection context which can be used outside this module
    to Send notificaitons to client.
*/
BLE_ctx get_ble_ctx();
