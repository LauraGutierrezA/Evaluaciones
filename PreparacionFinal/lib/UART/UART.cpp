#include "UART.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include <string.h>

Uart::Uart(uart_port_t UART_PORT, uint8_t TXD_PIN, uint8_t RDX_PIN, const char* config) {
    _uartPort = UART_PORT; 
    _txdPin = TXD_PIN;
    _rdxPin = RDX_PIN;
    _config = config; 
    _rxIndex = 0;
}

void Uart::init(void) {
    uart_word_length_t data_bits = UART_DATA_8_BITS;
    uart_parity_t parity = UART_PARITY_DISABLE;
    uart_stop_bits_t stop_bits = UART_STOP_BITS_1;

    int baud_rate = atoi(_config);
    char* ptrParity = strpbrk(_config, "NEOneo");
    if (ptrParity != NULL) {
        char data_bits_char = *(ptrParity - 1); 
        switch (data_bits_char) {
            case '5': data_bits = UART_DATA_5_BITS; break;
            case '6': data_bits = UART_DATA_6_BITS; break;
            case '7': data_bits = UART_DATA_7_BITS; break;
            case '8': data_bits = UART_DATA_8_BITS; break;
        }
            
        char stop_bits_char = *(ptrParity + 1);
        switch (stop_bits_char) {
            case '1': stop_bits = UART_STOP_BITS_1; break;
            case '2': stop_bits = UART_STOP_BITS_2; break;
        }

        switch (*ptrParity) {
            case 'N':
            case 'n':
                parity = UART_PARITY_DISABLE;
                break;
            case 'E':
            case 'e':
                parity = UART_PARITY_EVEN;
                break;
            case 'O':
            case 'o':
                parity = UART_PARITY_ODD;
                break;
        }

    }

    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));
    uart_config.baud_rate = baud_rate;
    uart_config.data_bits = data_bits;
    uart_config.parity = parity;
    uart_config.stop_bits = stop_bits;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    uart_param_config(_uartPort, &uart_config);
    uart_set_pin(_uartPort, _txdPin, _rdxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(_uartPort, 1024, 1024, 0, NULL, 0); //instala 1024 bytes para el Buffer ringf
}


Uart::~Uart() {
    uart_driver_delete(_uartPort);
}

void Uart::sendUART(const char* msg) {
    uart_write_bytes(_uartPort, msg, strlen(msg)); 
}

bool Uart::writeUART(const char* format, int*outValue) {
    uint8_t data = 0;
    int len = uart_read_bytes(_uartPort, &data, 1, pdMS_TO_TICKS(10));
    if (len > 0) {
        _rxBuffer[_rxIndex] = data; 
        _rxIndex++;

        if (_rxIndex >= 63) {
            _rxIndex = 0; 
        }

        if (data == '\n' || data == '\r') {
            _rxBuffer[_rxIndex] = '\0';
            int VarUpdated;
            _rxIndex = 0;
            if (sscanf(_rxBuffer, format, &VarUpdated) == 1) {
                *outValue = VarUpdated;
                const char *conf = "Variable Actualizada \n";
                uart_write_bytes(_uartPort, conf, strlen(conf));
                return true; //Funcionó, le avisa al main
            }

            
        }
    }
    return false; //no ha llegado el comando o no coincide con el formato esperado
}

char Uart::readCommandUART(void) {
    uint8_t data = 0;
    int len = uart_read_bytes(_uartPort, &data, 1, pdMS_TO_TICKS(10));
    if (len > 0) {
        return data;
    }
    return '\0';
}