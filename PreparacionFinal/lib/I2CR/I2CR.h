#ifndef __I2CR_H__
#define __I2CR_H__  

#include <stdint.h>
#include "driver/i2c.h"

struct RtcTime {
    uint8_t _seconds;
    uint8_t _minutes;
    uint8_t _hours;
    uint8_t _dayOfWeek;
    uint8_t _dayOfMonth;
    uint8_t _month;
    uint16_t _year;
};  

class I2cr {
    public: 
        I2cr(i2c_port_t I2C_PORT, uint8_t I2C_SDA_PIN, uint8_t I2C_SCL_PIN, uint32_t I2C_FREQ_HZ, uint8_t SLAVE_ADDR); 
        ~I2cr();
        void init(void);
        uint8_t decimal_a_bcd(uint8_t valor);
        uint8_t bcd_a_decimal(uint8_t valor);
        esp_err_t writeTime(RtcTime time);
        esp_err_t readTime(RtcTime *time);
        


    private: 
        i2c_port_t _i2cPort; 
        uint8_t _sdaPin;
        uint8_t _sclPin;
        uint32_t _clkSpeed;
        uint8_t _slaveAddr;
};


#endif // __I2CR_H__