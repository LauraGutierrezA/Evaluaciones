
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "SPIMAX.h"
#include "UART.h"

#define BTN_1 23
#define BTN_2 22
#define LED_1 5

void descomponerValor(int valor, uint8_t* digitos) {
    for (int i = 0; i < 4; i++) {
        digitos[i] = valor % 10;
        valor /= 10;
    }
}

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
    spimax.writeRegister(0x09, 0xFF); //Decode Mode
    spimax.writeRegister(0x0A, 0x0A); //Intensity
    spimax.writeRegister(0x0B, 0x04); //Scan Limit
    
    uart.sendUART("SPIMAX Initialized\r\n");


    while(1){
        const char* cmd = "DISP:%d";
        int outValue = 0;
        uint8_t digitos[4];
        for (uint16_t i = 0; i < 1000; i++) {
            outValue = i;
            descomponerValor(outValue, digitos); //Descompone el valor en dígitos individuales
            spimax.writeRegister(0x01, digitos[0]);
            spimax.writeRegister(0x02, digitos[1]); //Actualiza el dígito 0 del display con el valor recibido por UART
            spimax.writeRegister(0x03, digitos[2]);
            spimax.writeRegister(0x04, digitos[3]);
            uart.sendUART("Display Updated\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Pequeña pausa para evitar saturar el CPU
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Pausa antes de reiniciar el conteo
    }
    
    
}
