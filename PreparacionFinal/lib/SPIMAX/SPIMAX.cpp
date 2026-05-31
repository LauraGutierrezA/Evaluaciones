#include "SPIMAX.h"

Spimax::Spimax(spi_host_device_t host, int MOSI, int MISO, int clk, int cs_pin, int clock_speed_hz, int mode) {
    _host = host;
    _MOSI = MOSI;
    _MISO = MISO;
    _clk = clk;
    _cs_pin = cs_pin;
    _clock_speed_hz = clock_speed_hz;
    _mode = mode;
}

Spimax::~Spimax() {
    spi_bus_free(_host);
}

void Spimax::init() {
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = _MOSI;
    buscfg.miso_io_num = _MISO;
    buscfg.sclk_io_num = _clk;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    spi_bus_initialize(_host, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = _clock_speed_hz;
    devcfg.mode = _mode; // SPI mode
    devcfg.spics_io_num = _cs_pin;
    devcfg.queue_size = 1;  
    spi_bus_add_device(_host, &devcfg, &_spi);
};

void Spimax::writeRegister(uint8_t address, uint8_t data) {
    uint8_t buff[2]; //Siempre manda de a 2 Bytes
    buff[0] = address;
    buff[1] = data;
    spi_transaction_t t = {}; 
    t.length = 16;            // Vamos a intercambiar 16 bits (2 bytes) en total
    t.tx_buffer = buff;    // Lo que enviamos
    
    spi_device_transmit(_spi, &t);
}

uint8_t Spimax::readRegister(uint8_t reg) {
    uint8_t tx_buff[2];
    uint8_t rx_buff[2]; // Aquí guardaremos lo que responda el esclavo

    // 1. Armamos el buffer de transmisión
    // NOTA: Muchos sensores SPI exigen encender el bit más significativo (MSB) 
    // de la dirección para indicar que es una operación de LECTURA. 
    // Si tu datasheet lo pide, sería: tx_buff[0] = reg | 0x80;
    tx_buff[0] = reg;   
    
    // 2. El segundo byte es el "Dummy byte" para mantener el reloj activo
    tx_buff[1] = 0x00;  

    // 3. Configuramos la transacción a estilo C++ seguro
    spi_transaction_t t = {}; 
    t.length = 16;            // Vamos a intercambiar 16 bits (2 bytes) en total
    t.tx_buffer = tx_buff;    // Lo que enviamos
    t.rx_buffer = rx_buff;    // Donde guardamos lo que nos llega

    // 4. Ejecutamos la transferencia
    spi_device_transmit(_spi, &t);

    // rx_buff[0] contiene basura que llegó mientras enviábamos la dirección.
    // rx_buff[1] contiene el dato real que nos respondió el sensor.
    return rx_buff[1];
}