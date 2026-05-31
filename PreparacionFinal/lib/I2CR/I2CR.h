#ifndef __I2CR_H__
#define __I2CR_H__  

#include <stdint.h>
#include "driver/i2c.h"

struct RtcTime {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek;
    uint8_t dayOfMonth;
    uint8_t month;
    uint16_t year;
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