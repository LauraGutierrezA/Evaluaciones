#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "Ble.h"
#include "LED.h"

#define LED_PIN 2


extern "C" void app_main(void) {

    // Inicializamos BLE con nuestro nombre comercial
    Ble miBle("FINAL");
    miBle.init(); 

    Led ledAzul(LED_PIN);
    ledAzul.init();

    while (1) {
        if (miBle.hasData()) {
            char comando = miBle.getCommand(); 

            if (comando == 'E') {
                ledAzul.on();
                miBle.send("LED Encendido\r\n");
            } 
            else if (comando == 'A') {
                ledAzul.off();
                miBle.send("LED Apagado\r\n");
            } 
            
        }

        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}