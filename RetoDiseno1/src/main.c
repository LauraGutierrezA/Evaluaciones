#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define SAMPLE_PERIOD_US 100000

void app_main() {
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

    adc_cali_handle_t adc_cali_handle;

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1, 
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle); 

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
    
    uint64_t timer_value = 0; 
    int raw = 0;
    int adc_raw = 0; 
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &raw);
    int voltage_mv = 0;
    int porcentaje = 0;
    int Vmin_real = 142;
    int Vmax_real = 3139;

    while(1){
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_value); 

        if(timer_value >= SAMPLE_PERIOD_US){
            timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

            adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_raw);
            adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage_mv);

            porcentaje = ((voltage_mv-Vmin_real)*100)/(Vmax_real-Vmin_real);
            if(porcentaje > 100) porcentaje = 100; 
            if(porcentaje < 0) porcentaje = 0;

            printf("Crudo: %d | Calibrado %dmV | Velocidad Motor %d%%\n", adc_raw, voltage_mv, porcentaje);

        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }


}
