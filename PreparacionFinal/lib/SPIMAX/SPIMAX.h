#ifndef __SPIMAX_H__
#define __SPIMAX_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"

class Spimax {
    public:
        Spimax(spi_host_device_t host, int MOSI, int MISO, int clk, int cs_pin, int clock_speed_hz, int mode); //MOSI puede ser cualquiera
        ~Spimax();
        void init();
        void writeRegister(uint8_t address, uint8_t data);
        uint8_t readRegister(uint8_t reg);
        
    private:
        spi_host_device_t _host;
        int _MOSI;
        int _MISO;
        int _clk;
        int _cs_pin;
        int _clock_speed_hz;
        int _mode;

        spi_device_handle_t _spi;       
};

#endif