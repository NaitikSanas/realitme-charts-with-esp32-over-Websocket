#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "fonts.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "math.h"
#include "mpu6050_driver/mpu60x0.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"




int mpu_6050_i2c_slave_init(void);
void mpu6050_angle_computation_task(void *arg);
void ws_data_send_task(void);
void start_server (void);

#define CAL_SW 0