#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define BTN_L 22
#define BTN_R 23

#define SEG_A 17
#define SEG_B 16
#define SEG_C 25
#define SEG_D 33
#define SEG_E 32
#define SEG_F 18
#define SEG_G 19
#define CC 14
#define CD 26  
#define CU 27  

#define LED_L 13
#define LED_R 12 

#define RLY_L 4
#define RLY_R 5 

#define SAMPLE_PERIOD_US 100000
#define DEBOUNCE_TIME 200000

static volatile bool flag_btn_L = 0; 
static volatile bool flag_btn_R = 0; 

static volatile uint64_t last_isr_time_btnL = 0;
static volatile uint64_t last_isr_time_btnR = 0;

static volatile bool estado_led_L = 0;
static volatile bool estado_led_R = 0;

#define MOSTRAR_RAYAS 1000
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
    {0, 0, 0, 0, 0, 0, 1} //Raya
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

static inline void display(uint16_t valor) { 
    uint8_t decenas;
    uint8_t unidades;
    uint8_t centenas;

    if (valor == MOSTRAR_RAYAS) {
        centenas = 10;
        decenas = 10;  
        unidades = 10; 
    } else {
        if (valor > 999) valor = 999; 
        centenas = valor / 100;
        decenas = (valor / 10) % 10;
        unidades = valor % 10;
    }
    gpio_set_level(CD, 0);
    gpio_set_level(CU, 0);        
    escribir_segmentos(centenas);    
    gpio_set_level(CC, 1);          
    vTaskDelay(pdMS_TO_TICKS(3)); 

    gpio_set_level(CU, 0);
    gpio_set_level(CC, 0);        
    escribir_segmentos(decenas);    
    gpio_set_level(CD, 1);          
    vTaskDelay(pdMS_TO_TICKS(3)); 

    gpio_set_level(CC, 0);
    gpio_set_level(CD, 0);        
    escribir_segmentos(unidades);    
    gpio_set_level(CU, 1);          
    vTaskDelay(pdMS_TO_TICKS(3));  
}

// ISR para Botón L
static void IRAM_ATTR btnL_isr(void* arg){
    uint64_t current_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    if(current_time - last_isr_time_btnL >= DEBOUNCE_TIME){
        flag_btn_L = 1;
        last_isr_time_btnL = current_time;
    }
}

// ISR para Botón 2 
static void IRAM_ATTR btnR_isr(void* arg){
    uint64_t current_time;
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &current_time);
    if(current_time - last_isr_time_btnR >= DEBOUNCE_TIME){
        flag_btn_R = 1;
        last_isr_time_btnR = current_time;
    }
}

void app_main() {
    //Configuración ADC
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12 //rango 0-3.3c por atten 12db
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &chan_config); //Aquí no le digo en que GPIO estar si no que canal coger
    //Calibración
    adc_cali_handle_t adc_cali_handle;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1, 
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle); 

    //Configuración del PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0, 
        .duty_resolution = LEDC_TIMER_12_BIT, 
        .freq_hz = 5000, 
        .clk_cfg = LEDC_AUTO_CLK, 
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE, 
        .channel = LEDC_CHANNEL_0, 
        .timer_sel = LEDC_TIMER_0, 
        .intr_type = LEDC_INTR_DISABLE, 
        .gpio_num = 2, 
        .duty = 0, 
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    // Configuración del Timer
    timer_config_t timer_conf = {
		.divider = 80, 
		.counter_dir = TIMER_COUNT_UP, 
		.counter_en = TIMER_PAUSE, 
		.alarm_en = TIMER_ALARM_DIS, 
		.auto_reload = false
	}; 
	timer_init(TIMER_GROUP_0,  TIMER_0, &timer_conf);
	timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0); 
	timer_start(TIMER_GROUP_0, TIMER_0);


    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL<< SEG_A) | (1ULL<< SEG_B) | (1ULL<< SEG_C) | 
                        (1ULL<< SEG_D) | (1ULL<< SEG_E) | (1ULL<< SEG_F) | 
                        (1ULL<< SEG_G) | (1ULL<< CU) | (1ULL<< CD) | (1ULL<< CC) | 
                        (1ULL<< LED_L) | (1ULL<< LED_R) |
                        (1ULL<< RLY_L) | (1ULL<< RLY_R),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_cfg); 
    gpio_set_level(CD, 0); gpio_set_level(CU, 0); 
    gpio_set_level(LED_L, 0); gpio_set_level(LED_R, 0);

    gpio_config_t in_cfg = {
        .pin_bit_mask = (1ULL << BTN_L) | (1ULL << BTN_R),
        .mode = GPIO_MODE_INPUT, 
        .pull_up_en = GPIO_PULLUP_ENABLE, // NO en 6 ni 11
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&in_cfg);

    gpio_install_isr_service(0); 
    gpio_isr_handler_add(BTN_L, btnL_isr, NULL); 
    gpio_isr_handler_add(BTN_R, btnR_isr, NULL); 
    
    uint64_t now = 0; 
    uint64_t last = 0;

    int raw = 0;
    int adc_raw = 0; 
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &raw);
    int voltage_mv = 0;
    int porcentaje = 0;
    int Vmin_real = 142;
    int Vmax_real = 3139;

    // FSM
    typedef enum {
        E_INICIO, 
        E_L, 
        E_R
    } estado_t; 
    
    estado_t estado_actual = E_INICIO;

    while(1){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &now); 
        display(porcentaje);

        if((now - last) >= SAMPLE_PERIOD_US){
            last = now;

            adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_raw);
            adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage_mv);

            porcentaje = ((voltage_mv-Vmin_real)*100)/(Vmax_real-Vmin_real);
            if(porcentaje > 100) porcentaje = 100; 
            if(porcentaje < 0) porcentaje = 0;

            printf("Crudo: %d | Calibrado %dmV | Velocidad Motor %d%%\n", adc_raw, voltage_mv, porcentaje);

        }
        vTaskDelay(pdMS_TO_TICKS(10));


        uint32_t duty_cycle = (porcentaje * 4095)/100;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        switch (estado_actual){
            case E_INICIO:
                gpio_set_level(LED_L, 0);
                gpio_set_level(LED_R, 0);

                gpio_set_level(RLY_L, 0);
                gpio_set_level(RLY_R, 0);

                if(flag_btn_L){
                    flag_btn_L = 0;
                    gpio_set_level(LED_L, 1); 

                    gpio_set_level(RLY_R, 0);
                    gpio_set_level(RLY_L, 1);

                    estado_actual = E_L;
                }
                else if(flag_btn_R){
                    flag_btn_R = 0;
                    gpio_set_level(LED_R, 1); 

                    gpio_set_level(RLY_L, 0);
                    gpio_set_level(RLY_R, 1);

                    estado_actual = E_R;
                }
                break;
            case E_L:
                if (flag_btn_L) {
                    flag_btn_L = 0;
                    estado_actual = E_INICIO;
                }
                else if (flag_btn_R){
                    flag_btn_R = 0;
                }
                break;
            case E_R:
                if (flag_btn_R) {
                    flag_btn_R = 0;
                    estado_actual = E_INICIO;
                }
                else if (flag_btn_L){
                    flag_btn_L = 0;
                }
                break;
        

        }


    }


}
