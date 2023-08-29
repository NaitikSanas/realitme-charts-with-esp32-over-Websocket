#include "ble_app_thread.h"

void ble_app(void)
{
    esp_err_t ret;
    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    /* 
        BLE Initialization
    */
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    /*
        BLE Event Handler and App profiler Registering
    */
    esp_ble_gatts_register_callback(gatts_event_handler); //register gatt server event handler
    esp_ble_gap_register_callback(gap_event_handler);     //register GAP Event handler
    esp_ble_gatts_app_register(PROFILE_A_APP_ID);         //register GATT Server Profile
   
    esp_ble_gatt_set_local_mtu(500);
    return;
}