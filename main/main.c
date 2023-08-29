#include "main.h"
#include "websocket_server_api/websocket_server.h"
#include "wifi/wifi.h"   
static const char *TAG = "MAIN";

mpu_handle_t mpu;
int acc_error=0;                         //We use this variable to only calculate once the Acc data error
char str[64] = {0};
int ws_data_received_cb(uint8_t* data, size_t len){
    printf("From Main");
    return 0;   
}
uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax, uint32_t au32_OUTmin, uint32_t au32_OUTmax)
{
    return ((((au32_IN - au32_INmin)*(au32_OUTmax - au32_OUTmin))/(au32_INmax - au32_INmin)) + au32_OUTmin);
}

void app_main(){
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();  
    start_server();

    gpio_pad_select_gpio(CAL_SW);
    gpio_set_direction(CAL_SW,GPIO_MODE_INPUT);
    gpio_pullup_en(CAL_SW);
    
    mpu_6050_i2c_slave_init();
    SSD1306_Init();
   
    xTaskCreate(mpu6050_angle_computation_task, "mpu6050_angle_computation_task_1", 1024 * 2, (void *)1, 10, NULL);
    xTaskCreate(ws_data_send_task, "ws_data_send_tasks", 1024 * 2, (void *)1, 10, NULL);
}

int mpu_6050_i2c_slave_init(void)
{
    esp_err_t err;
    mpu.addr = 104;
    mpu.bus.num = 0;
    mpu.bus.timeout = 50;
    mpu.init.clk_speed = 400000;
    mpu.init.sda_io_num = 6;
    mpu.init.scl_io_num = 4;
    mpu.init.sda_pullup_en = false;
    mpu.init.scl_pullup_en = false;
    init : err = mpu_initialize_peripheral(&mpu);
    printf("Returned with %x", (int)err);
    if(err != ESP_OK)goto init;
    ESP_LOGI(TAG, "MPU connection successful!");

    err = mpu_initialize_chip(&mpu);
    ESP_LOGI(TAG, "i2c_slave_init");
    return 0;
}

char data[256] = {0};
float Acc_angle_z = 0,  Acc_angle_x = 0, Acc_angle_y =0;          //Here we store the angle value obtained with Acc data
raw_axes_t accel_raw;   // x, y, z axes as int16

void ws_data_send_task(void){
    while (1)
    {   
        memset(data,0,256);
        sprintf(data,"{\"x\" : [\"%.1f\"],\"y\":[\"%.1f\"],\"z\":[\"%.1f\"],\"x_acc\": [\"%d\"],\"y_acc\":[\"%d\"],\"z_acc\":   [\"%d\"]}",
                        Acc_angle_x,Acc_angle_y,Acc_angle_z, accel_raw.x, accel_raw.y , accel_raw.z);
        ws_set_data_fields((uint8_t*)data,strlen(data),false);
        vTaskDelay(1);
    } 
}

void mpu6050_angle_computation_task(void *arg)
{
    esp_err_t err;
    // Reading sensor data
    printf("Reading sensor data:\n");
    
    raw_axes_t gyro_raw;    // x, y, z axes as int16
    float_axes_t accel_g;   // accel axes in (g) gravity format
    float_axes_t gyro_dps;  // gyro axes in (DPS) º/s format
    int offset_x = 0, offset_y = 0;

    
    while (true)
    {
        // Read
        mpu_acceleration(&mpu, &accel_raw);  // fetch raw data from the registers
        err = mpu_rotation(&mpu, &gyro_raw);       // fetch raw data from the registers
        mpu_motion(&mpu, &accel_raw, &gyro_raw);  // read both in one shot
        // Convert
        accel_g = mpu_math_accel_gravity(&accel_raw, ACCEL_FS_4G);
        gyro_dps = mpu_math_gyro_deg_per_sec(&gyro_raw, GYRO_FS_500DPS);

        //Acc Variables
        float rad_to_deg = 180/3.141592654;      //This value is for pasing from radians to degrees values
        float Acc_rawX, Acc_rawY, Acc_rawZ;    //Here we store the raw data read 
       
        float Acc_angle_error_x = 0, Acc_angle_error_y = 0,  Acc_angle_z_error = 0; //Here we store the initial Acc data error
        
        if(acc_error==0)
        {
            for(int a=0; a<200; a++)
            {
                err = mpu_acceleration(&mpu, &accel_raw);  // fetch raw data from the registers
                ESP_ERROR_CHECK(err);
                err = mpu_rotation(&mpu, &gyro_raw);       // fetch raw data from the registers
                ESP_ERROR_CHECK(err);

                mpu_motion(&mpu, &accel_raw, &gyro_raw);  // read both in one shot

                Acc_rawX = accel_raw.x;
                Acc_rawY = accel_raw.y;
                Acc_rawZ = accel_raw.z;
                
                /*---X---*/
                Acc_angle_error_x = Acc_angle_error_x + ((atan((Acc_rawY)/sqrt(pow((Acc_rawX),2) + pow((Acc_rawZ),2)))*rad_to_deg));
                /*---Y---*/
                Acc_angle_error_y = Acc_angle_error_y + ((atan(-1*(Acc_rawX)/sqrt(pow((Acc_rawY),2) + pow((Acc_rawZ),2)))*rad_to_deg)); 

                Acc_angle_z_error = Acc_angle_z_error + ((180/3.141592) * atan(sqrt(pow(accel_raw.z,2)) + pow(accel_raw.y,2) /accel_raw.z));
                if(a==199)
                {
                    Acc_angle_error_x = Acc_angle_error_x/200;
                    Acc_angle_error_y = Acc_angle_error_y/200;
                    acc_error=1;
                }
                acc_error = 1;
            }
            printf("Errors : x->%f y->%f\n",Acc_angle_error_x,Acc_angle_error_y);
        }//end of acc error calculation   


         /*---X---*/

        Acc_angle_x = (atan((accel_raw.x)/sqrt(pow((accel_raw.y),2) + pow((accel_raw.z),2)))*rad_to_deg) - Acc_angle_error_x;
 /*---Y---*/
        Acc_angle_y = (atan(-1*(accel_raw.y)/sqrt(pow((accel_raw.x),2) + pow((accel_raw.z),2)))*rad_to_deg) - Acc_angle_error_y;    

        Acc_angle_z = ((180/3.141592) * atan(sqrt(pow(accel_raw.z,2)) + pow(accel_raw.y,2) /accel_raw.z)) - Acc_angle_z_error;
        // printf(" X : %f, Y : %f, Z : %f\n", Acc_angle_x, Acc_angle_y,Acc_angle_z);
        
        
        
        
        SSD1306_Clear();
        int px = MAP(Acc_angle_x, -90, 90, 0, 128);
        int py = MAP(Acc_angle_y, -90, 90, 0, 64);
        int pz = MAP(Acc_angle_z,-90,90,0,128);
        /*
            calibrate dot to p(x,y) (64, 32) 
       */
        if(gpio_get_level(CAL_SW)==0){
            while (gpio_get_level(CAL_SW)==0)vTaskDelay(1);
            printf("Calibrating Now...\n");
            offset_x = 64 - px;
            offset_y = 32 - py;
            
            printf("Offsets : x = %d, y =%d\n", offset_x, offset_y);
        }
        if(offset_x < 0) px = px - abs(offset_x);
        else if(offset_x > 0) px = px + offset_x;
        if(offset_y < 0) py = py - abs(offset_y);
        else py = py + offset_y;

        SSD1306_DrawCircle(px,py,5,1); 
      //  printf("PX=%d,PY=%d\n", px,py);       
        SSD1306_GotoXY(0,0);
        SSD1306_Puts("DEMO",&Font_7x10,1);


        sprintf(str,"PX=%d,PY=%d", px,py);        
        SSD1306_GotoXY(0,18);
        SSD1306_Puts(str,&Font_7x10,1);

        sprintf(str, "Acc:%.1f,%.1f,%.1f", Acc_angle_x, Acc_angle_y, Acc_angle_z);
        SSD1306_GotoXY(0,10+18 );
        SSD1306_Puts(str,&Font_7x10,1);
        SSD1306_UpdateScreen();

        
       
       // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}