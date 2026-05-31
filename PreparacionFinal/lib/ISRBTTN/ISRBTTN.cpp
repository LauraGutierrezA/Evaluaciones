#include "ISRBTTN.h"
#include "freertos/FreeRTOS.h" 

ISRBTTN::ISRBTTN(uint8_t pin, uint32_t debounce_us) {
    _pin = pin;
    _debounce_time = debounce_us;
    _flag = false;
    _last_isr_time = 0;
}

void ISRBTTN::init() {
    // Configuración de pin como entrada y con pull-up
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << _pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // Interrupción en flanco de bajada
    gpio_config(&io_conf);

    // Configuramos la ISR para el pin
    gpio_isr_handler_add((gpio_num_t)_pin, isr_handler, (void*)this);
}

// Interrupción 
void IRAM_ATTR ISRBTTN::isr_handler(void* arg) {
    // Como es estática, recuperamos nuestro objeto usando el argumento
    ISRBTTN* btn = (ISRBTTN*)arg;

    // Usamos el reloj del sistema en microsegundos
    uint64_t current_time = esp_timer_get_time(); 
    
    // Lógica de debounce
    if (current_time - btn->_last_isr_time >= btn->_debounce_time) {
        btn->_flag = true;
        btn->_last_isr_time = current_time;
    }
}

// Para el main
bool ISRBTTN::isPressed() {
    if (_flag) {
        _flag = false; // Auto-limpiamos la bandera 
        return true;
    }
    return false;
}