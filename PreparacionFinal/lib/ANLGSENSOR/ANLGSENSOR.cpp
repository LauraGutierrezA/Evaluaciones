#include "ANLGSENSOR.h"

AnlgSensor::AnlgSensor(adc_unit_t unit, adc_channel_t channel) {
    _unit = unit;
    _channel = channel;
    adc_oneshot_unit_handle_t _adcHandle;
}

AnlgSensor::~AnlgSensor() {
    adc_oneshot_del_unit(_adcHandle);
}

void AnlgSensor::init(void) {
  adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = _unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config, &_adcHandle);

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(_adcHandle, _channel, &chan_config);
}

int AnlgSensor::read(void) {
    int adc_value;
    adc_oneshot_read(_adcHandle, _channel, &adc_value);
    return adc_value;
}