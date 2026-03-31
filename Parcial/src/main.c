#include <stdint.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "driver/gpio.h"

#define BTN_1 23
#define BTN_2 22
#define BTN_3 21

#define SEG_A 17
#define SEG_B 16
#define SEG_C 25
#define SEG_D 33
#define SEG_E 32
#define SEG_F 18
#define SEG_G 19
#define CD 26  
#define CU 27  

#define LED 14

#define T_TRANSICION       1000000  // 1 Segundo
#define DEBOUNCE_TIME 200000   // 200 ms
#define TOTAL_PARPADEO 4000000 //4s
#define T_PARPADEO    500000 
#define MOSTRAR_RAYAS 100

static volatile bool flag_btn1 = 0; 
static volatile bool flag_btn2 = 0; 
static volatile bool flag_btn3 = 0; 

static volatile uint64_t last_isr_time_btn1 = 0;
static volatile uint64_t last_isr_time_btn2 = 0;
static volatile uint64_t last_isr_time_btn3 = 0;

static volatile bool estado_led = 0;

const uint8_t matriz_numeros[11][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1},  // 9
    {0, 0, 0, 0, 0, 0, 1}  // Rayas


};

static inline void escribir_segmentos(uint8_t n) {
    gpio_set_level(SEG_A, matriz_numeros[n][0]);
    gpio_set_level(SEG_B, matriz_numeros[n][1]);
    gpio_set_level(SEG_C, matriz_numeros[n][2]);
    gpio_set_level(SEG_D, matriz_numeros[n][3]);
    gpio_set_level(SEG_E, matriz_numeros[n][4]);
    gpio_set_level(SEG_F, matriz_numeros[n][5]);
    gpio_set_level(SEG_G, matriz_numeros[n][6]);
}

// Función de Multiplexación
static inline void display(uint8_t valor) {
    uint8_t decenas;
    uint8_t unidades;
    if(valor == MOSTRAR_RAYAS){
        decenas = 10; 
        unidades = 10;
    }
    else {
        if (valor > 99) valor = 99;  

         decenas = valor / 10;
         unidades = valor % 10;
    }
	//DECENAS
    gpio_set_level(CU, 0);          
    escribir_segmentos(decenas);    
    gpio_set_level(CD, 1);          
    vTaskDelay(pdMS_TO_TICKS(10));  

    //UNIDADES
    gpio_set_level(CD, 0);          
    escribir_segmentos(unidades);   
    gpio_set_level(CU, 1);          
    vTaskDelay(pdMS_TO_TICKS(10));  
}

// ISR para Botón 1 
static void IRAM_ATTR btn1_isr(void* arg){
    uint64_t current_time = 0;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    if(current_time - last_isr_time_btn1 >= DEBOUNCE_TIME){
        flag_btn1 = 1;
        last_isr_time_btn1 = current_time;
    }
}

// ISR para Botón 2 
static void IRAM_ATTR btn2_isr(void* arg){
    uint64_t current_time = 0;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    if(current_time - last_isr_time_btn2 >= DEBOUNCE_TIME){
        flag_btn2 = 1;
        last_isr_time_btn2 = current_time;
    }
}

// ISR para Botón 3
static void IRAM_ATTR btn3_isr(void* arg){
    uint64_t current_time = 0;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    if(current_time - last_isr_time_btn3 >= DEBOUNCE_TIME){
        flag_btn3 = 1;
        last_isr_time_btn3 = current_time;
    }
}

// ISR de Parpadeo (timer) 
static bool IRAM_ATTR alarma_isr(void* arg){
    estado_led = !estado_led;
    gpio_set_level(LED, estado_led);
    return false; 
}

void app_main() {
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL<< SEG_A) | (1ULL<< SEG_B) | (1ULL<< SEG_C) | 
                        (1ULL<< SEG_D) | (1ULL<< SEG_E) | (1ULL<< SEG_F) | 
                        (1ULL<< SEG_G) | (1ULL<< CU) | (1ULL<< CD) | 
                        (1ULL<< LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_cfg); 
    gpio_set_level(CD, 0); gpio_set_level(CU, 0); 
    gpio_set_level(LED, 0); 

    gpio_config_t in_cfg = {
        .pin_bit_mask = (1ULL << BTN_1) | (1ULL << BTN_2) | (1ULL << BTN_3),
        .mode = GPIO_MODE_INPUT, 
        .pull_up_en = GPIO_PULLUP_ENABLE, // NO en 6 ni 11
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&in_cfg);

    gpio_install_isr_service(0); 
    gpio_isr_handler_add(BTN_1, btn1_isr, NULL); 
    gpio_isr_handler_add(BTN_2, btn2_isr, NULL);
    gpio_isr_handler_add(BTN_3, btn3_isr, NULL); 

    // Timer de Debouncing
    timer_config_t timer_conf_main = {
        .divider = 80, 
        .counter_dir = TIMER_COUNT_UP, 
        .counter_en = TIMER_PAUSE, 
        .alarm_en = TIMER_ALARM_DIS, 
        .auto_reload = false
    }; 
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf_main); 
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0); 
    timer_start(TIMER_GROUP_0, TIMER_0);

    // Timer 1: Alarmas/blinking 
    timer_config_t timer_conf_alarma = {
        .divider = 80, 
        .counter_dir = TIMER_COUNT_UP, 
        .counter_en = TIMER_PAUSE, 
        .alarm_en = TIMER_ALARM_EN, 
        .auto_reload = true
    };
    timer_init(TIMER_GROUP_0, TIMER_1, &timer_conf_alarma); 
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0); 
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, T_PARPADEO);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_1, alarma_isr, NULL, 0);
    timer_enable_intr(TIMER_GROUP_0, TIMER_1); 
    
    uint64_t now= 0;
	uint64_t last = 0; 

    uint8_t estacion_actual= 1;
	uint8_t estacion_destino = 1; 
    uint16_t numero_mostrar = 0;

    typedef enum {
        E_INICIAL, E_MOVIENDO, E_LLEGADA
    } estado_t; 
    estado_t estado_actual = E_INICIAL;


    
    while(1){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &now);
        switch(estado_actual){
            case E_INICIAL:
                display(MOSTRAR_RAYAS); 
                gpio_set_level(LED, 0);
                if(flag_btn1){
                    flag_btn1 = 0; 
                    estacion_destino = 1;
                    estado_actual = E_MOVIENDO;
                    last = now;
                }
                else if(flag_btn2){
                    flag_btn2 = 0; 
                    estacion_destino = 2;
                    estado_actual = E_MOVIENDO;
                    last = now;
                }
                else if(flag_btn3){
                    flag_btn3 = 0; 
                    estacion_destino = 3;
                    estado_actual = E_MOVIENDO;
                    last = now;
                }
                break; 
            case E_MOVIENDO: 
                
                
                if(now - last >= T_TRANSICION){
                    
                    last = now; 

                    if(estacion_actual >= 3){
                        estacion_actual = 1;
                    }
                    else{
                        estacion_actual = estacion_actual +1;
                    }
                    
 
                }
                numero_mostrar = estacion_actual + estacion_destino*10;
                display(numero_mostrar);

                
                if(flag_btn1)flag_btn1 = 0;
                if(flag_btn2)flag_btn2 = 0;
                if(flag_btn3)flag_btn3 = 0;

                if(estacion_actual == estacion_destino){
                    timer_start(TIMER_GROUP_0, TIMER_1);
                    estado_actual = E_LLEGADA;
                    last = now;
                }
                break;

            case E_LLEGADA:
                display(numero_mostrar);
                if(now - last >= TOTAL_PARPADEO){
                    timer_pause(TIMER_GROUP_0, TIMER_1);
                    last = now;
                    estado_actual =  E_INICIAL;
                }
                break;
            



            

        }

    }

}