idf_component_register(SRCS "wifi/wifi.c" "dashboard_server.c" 
                            "mpu6050_driver/mpu60x0.c" "mpu6050_driver/mpu60x0_i2c.c" 
                            "main.c" "websocket_server_api/websocket_server.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "web_interface/chart.js"
                                "web_interface/index.html"
                                "web_interface/index.js"
)