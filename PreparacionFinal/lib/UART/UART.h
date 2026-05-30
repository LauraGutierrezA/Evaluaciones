#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include "driver/uart.h"


class Uart {
    public: 
        Uart(uart_port_t UART_PORT, uint8_t TXD_PIN, uint8_t RDX_PIN, const char* config); 
        ~Uart();
        void init(void);
        void sendUART(const char* msg); 
        bool writeUART(const char* format, int*outValue); 
        char readCommandUART(void);
    private: 
        uart_port_t _uartPort; 
        uint8_t _txdPin;
        uint8_t _rdxPin;
        const char* _config;
        char _rxBuffer[64];
        uint8_t _rxIndex;

};

#endif


