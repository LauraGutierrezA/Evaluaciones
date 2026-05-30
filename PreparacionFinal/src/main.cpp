#include "UART.h"
#include "LED.h"

Led ledAzul(25); // GPIO 2 para el LED
Uart uart(UART_NUM_0, 1, 3, "115200 8N1");

const char *MSG_INICIO = "\nUART Iniciado\n"; 

extern "C" void app_main() {
    ledAzul.init();
    ledAzul.off();
    uart.init();       
    uart.sendUART(MSG_INICIO);   

    int brightness = 100;
   

    ledAzul.breathe(10);
    ledAzul.init();
    ledAzul.off();


    while (1){ 
        
        char comando = uart.readCommandUART();

        if( comando == 'E'){
            ledAzul.init();
            uart.sendUART("Encendiendo LED\n");
            ledAzul.on();
        }
        else if( comando == 'A'){
            ledAzul.init();
            uart.sendUART("Apagando LED\n");
            ledAzul.off();
        }
        else if( comando == 'T'){
            ledAzul.init();
            uart.sendUART("Toggling LED\n");
            ledAzul.toggle(5);
        }
        else if( comando == 'B'){
            uart.sendUART("Breathe LED\n");
            ledAzul.breathe(10);
            ledAzul.off();
        }
        else if( comando == 'I'){
            brightness += 100;
            if (brightness > 1023) brightness = 0;
            uart.sendUART("Ajustando Intensidad\n");
            ledAzul.intensity(brightness);
        }


        vTaskDelay(pdMS_TO_TICKS(10));
    }

}