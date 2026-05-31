#include "TIMERASDELAY.h"
#include "ANLGSENSOR.h"
#include "UART.h"

extern "C" void app_main(void) {
    // Inicialización de componentes
    TimerAsDelay timer(TIMER_GROUP_0, TIMER_0);
    timer.init();

    AnlgSensor sensor(ADC_UNIT_1, ADC_CHANNEL_4);
    sensor.init();

    Uart uart( UART_NUM_0, 1, 3, "115200N81" ); 
    uart.init();

    uart.sendUART("Sistema iniciado. Leyendo sensor...\n");

    while (1) {
        int valor = sensor.read();

        char mensaje_salida[50]; 
        snprintf(mensaje_salida, sizeof(mensaje_salida), "Valor del sensor: %d\n", valor);
        
        if (timer.delay_s(1)) {
           uart.sendUART(mensaje_salida);
        }
    }
}