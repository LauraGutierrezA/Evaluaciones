#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_rom_sys.h"

//Pines UART
#define UART_PORT UART_NUM_0 //Porque 0 es el del USB
#define TXD_PIN 1
#define RDX_PIN 3


adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc_cali_handle;

void app_main(void){
    vTaskPrioritySet(NULL, 0); // Para no asfixiar al WDT
	uart_config_t uart_config = {
		.baud_rate = 9600, //El único que se toca por si necesito más velocidad
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE, 
		.stop_bits = UART_STOP_BITS_1, 
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE, 
		.source_clk = UART_SCLK_DEFAULT,
	};
	uart_param_config(UART_NUM_0, &uart_config); // Puedo usar NUM_0, 1 o 2
	uart_set_pin(UART_NUM_0, TXD_PIN, RDX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); //Cambiamos TDX_PIN y RDX_pin por el pin que queremos
	uart_driver_install(UART_NUM_0, 1024, 1024, 0, NULL, 0);//inicializa el módulo del ESP. 
	//los 1024 son buffers que son vectores (en este caso de 1024 bits por defecto). Si el sistema tiene muy poquitos recursos, ahí si calcula
	
    adc_oneshot_unit_init_cfg_t init_config = {
	    .unit_id = ADC_UNIT_1
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12 //rango 0-3.3c por atten 12db
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &chan_config); //LM35
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &chan_config); //LDR
    

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1, 
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle); 

    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL<< IN1) | (1ULL<< IN2) | (1ULL<< IN3) | (1ULL<<IN4) |
                        (1ULL<<CALEF),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_cfg); 
    motor_apagar();

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
        .gpio_num = 22, 
        .duty = 0, 
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);




	while(1) {
        

	};
}