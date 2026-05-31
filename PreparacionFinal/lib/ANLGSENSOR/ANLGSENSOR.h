#ifndef __ANLGSENSOR_H__
#define __ANLGSENSOR_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_oneshot.h"

class AnlgSensor {
    public:
        AnlgSensor(adc_unit_t unit, adc_channel_t channel);
        ~AnlgSensor();
        void init(void);
        int read(void);

    private:
        adc_unit_t _unit;
        adc_channel_t _channel;
        adc_oneshot_unit_handle_t _adcHandle;
};

#endif // __ANLGSENSOR_H__