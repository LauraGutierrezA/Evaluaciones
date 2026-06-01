#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "BTC.h" // Incluimos tu nueva clase compacta
#include "LED.h"

#define LED_PIN GPIO_NUM_2

extern "C" void app_main(void) {
    // Configuración inicial del LED físico
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
    gpio_set_level(LED_PIN, 0);

    // Creamos nuestro objeto Bluetooth y le damos el nombre que verá el celular
    Btc bluetooth("Mi_ESP32_Reloj");
    bluetooth.init(); // Arranca toda la magia asincrónica en paralelo

    while (1) {
        // En nuestro ciclo infinito, solo le preguntamos al objeto si el callback detectó algo
        if (bluetooth.hasData()) {
            char comando = bluetooth.getCommand(); // Obtenemos el comando

            if (comando == 'E') {
                gpio_set_level(LED_PIN, 1);
                bluetooth.send("Acción ejecutada: LED Encendido desde C++\r\n");
            } 
            else if (comando == 'A') {
                gpio_set_level(LED_PIN, 0);
                bluetooth.send("Acción ejecutada: LED Apagado desde C++\r\n");
            } 
            else {
                bluetooth.send("Comando no reconocido.\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Pausa pequeña para descanso del procesador
    }
}