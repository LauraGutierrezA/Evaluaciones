#ifndef __ISRBTTN_H__
#define __ISRBTTN_H__

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_timer.h" // Para usar el reloj del sistema (esp_timer_get_time)

class ISRBTTN {
public:
    ISRBTTN(uint8_t pin, uint32_t debounce_us);
    void init();
    bool isPressed(); 

private:
    uint8_t _pin;
    uint32_t _debounce_time;
    
    volatile bool _flag;
    volatile uint64_t _last_isr_time;

    static void  isr_handler(void* arg);
};

#endif