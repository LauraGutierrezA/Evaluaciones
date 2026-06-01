#ifndef __SPIMCP_H__
#define __SPIMCP_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"

class Spimcp {
    public:
        Spimcp(spi_host_device_t host, int sclk_io_num, int mosi_io_num, int miso_io_num, int cs_pin, int clock_speed_hz, int mode);
        ~Spimcp();

        void spi_bus_init();
        
        void mcp4132_write_register(uint8_t address, uint8_t value);
        uint8_t mcp4132_read_register(uint8_t reg);



    private:
        spi_device_handle_t _spi_handle;
        spi_host_device_t _host;
        int _MOSI;
        int _MISO; 
        int _SCLK;
        int _CS;
        int _clock_speed_hz;
        int _mode;

};


#endif