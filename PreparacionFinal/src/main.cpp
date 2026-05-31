#include "I2CR.h"
#include "UART.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 32  
#define I2C_SCL_PIN 33
#define I2C_FREQ_HZ 100000
#define I2C_SLAVE_ADDR 0x68

#define UART_PORT UART_NUM_0
#define UART_TXD_PIN 1
#define UART_RDX_PIN 3
#define UART_CONFIG "115200N81"

extern "C" void app_main() {
    I2cr rtc(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ, I2C_SLAVE_ADDR);
    Uart uart(UART_PORT, UART_TXD_PIN, UART_RDX_PIN, UART_CONFIG);

    rtc.init();
    uart.init();
    //formato segundos, minutos, horas, día de la semana, día del mes, mes, año
    RtcTime horaActual = {30, 58, 11, 7, 31, 5, 25};
    rtc.writeTime(horaActual); // Escribe una hora específica
    RtcTime rtcTime;

    while(1){
        esp_err_t lectura = rtc.readTime(&rtcTime); // Lee la hora actual del RTC

        if (lectura == ESP_OK) {
            char buffer[100];
            const char* dayName = "Desconocido";
            switch (rtcTime.dayOfWeek) {
                case 1: dayName = " Lunes"; break;
                case 2: dayName = " Martes"; break;
                case 3: dayName = " Miércoles"; break;
                case 4: dayName = " Jueves"; break;
                case 5: dayName = " Viernes"; break;
                case 6: dayName = " Sábado"; break;
                case 7: dayName = " Domingo"; break;
            };
            snprintf(buffer, sizeof(buffer), "Hora: %02d:%02d:%02d, Fecha: %02d/%02d/%04d, Día de la semana: %s",
                    rtcTime.hours, rtcTime.minutes, rtcTime.seconds,
                    rtcTime.dayOfMonth, rtcTime.month, rtcTime.year,
                    dayName);
            uart.sendUART(buffer); // Envía la hora leída por UART
        } else {
            uart.sendUART("Error al leer el RTC");
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo antes de la siguiente lectura

    };
    
}