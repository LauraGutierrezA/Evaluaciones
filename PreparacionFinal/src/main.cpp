
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "SPIMAX.h"
#include "UART.h"

#define BTN_1 23
#define BTN_2 22
#define LED_1 5

extern "C" void app_main() {
    Uart uart( UART_NUM_0, 1, 3, "115200N81" ); 
    uart.init();

    spi_host_device_t host = SPI2_HOST;
    int MOSI = 23;
    int MISO = 22;
    int clk = 19;
    int cs_pin = 18;
    int clock_speed_hz = 10000000; //10MHz
    int mode = 0;

    Spimax spimax(host, MOSI, MISO, clk, cs_pin, clock_speed_hz, mode);
    spimax.init();

    spimax.writeRegister(0x0C, 0x01); //No shutdown
    spimax.writeRegister(0x0C, 0x01); //Test
    vTaskDelay(pdMS_TO_TICKS(3000));
    spimax.writeRegister(0x0C, 0x00); //Test

    
    
}
