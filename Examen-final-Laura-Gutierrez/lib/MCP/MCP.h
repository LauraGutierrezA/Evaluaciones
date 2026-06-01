#ifndef __MCP_H__
#define __MCP_H__

#include "freertos/FreeRTOS.h"
#include "esp_task.h"
#include "SPI.h"

class Mcp {
    public:
        Mcp();
        ~Mcp();

        void init();
        void mcp4132_set_wiper(uint8_t wiper);
        uint8_t mcp4132_set_cutoff_frequency(uint8_t frequency);

    private:
        uint8_t _wiper;
        Spimcp _spi;
};

#endif 