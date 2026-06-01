#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MCP.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "driver/gptimer.h"
#include "esp_adc_cal.h"
#include "MCP.h"


#define UART_PORT       UART_NUM_0
#define UART_BAUD       115200
#define ADC_CHANNEL     ADC1_CHANNEL_6   
#define ADC_ATTEN       ADC_ATTEN_DB_11  
#define ADC_WIDTH       ADC_WIDTH_BIT_12 /

#define TIMER_FREQ_HZ   1000             

#define V_HIGH_MV       1400            
#define V_LOW_MV         900             
#define WIPER_HIGH        95
#define WIPER_LOW         42


static volatile bool     s_sample_ready = false;
static esp_adc_cal_characteristics_t s_adc_chars;
static Mcp g_mcp;

static bool IRAM_ATTR timer_cb(gptimer_handle_t timer,
                               const gptimer_alarm_event_data_t *edata,
                               void *user_ctx)
{
    s_sample_ready = true;
    return false;   
}


static void uart_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_PORT, &cfg);
    uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0);
}


static void adc_init(void)
{
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN,
                             ADC_WIDTH, VREF_MV, &s_adc_chars);
}



extern "C" void app_main(void)
{
    uart_init();
    adc_init();
    g_mcp.init();
    timer_init();

    char buf[64];

    while (true)
    {
       //flag del timer

        // Leer y limpiar ADC
        int     raw    = adc1_get_raw(ADC_CHANNEL);
        uint32_t mv    = esp_adc_cal_raw_to_voltage(raw, &s_adc_chars);

        // Controlar Wiper según umbral
        if (mv > V_HIGH_MV) {
            g_mcp.mcp4132_set_wiper(WIPER_HIGH);          // N = 95
        } else if (mv < V_LOW_MV) {
            g_mcp.mcp4132_set_wiper(WIPER_LOW);           // N = 42
        }

        
    }
}