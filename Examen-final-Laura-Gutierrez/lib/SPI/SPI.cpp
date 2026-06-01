#include "SPI.h"

Spimcp::Spimcp(spi_host_device_t host, int sclk_io_num, int mosi_io_num, int miso_io_num, int cs_pin, int clock_speed_hz, int mode) {
    _host = host;
    _SCLK = sclk_io_num;
    _MOSI = mosi_io_num;
    _MISO = miso_io_num;
    _CS = cs_pin;
    _clock_speed_hz = clock_speed_hz;
    _mode = mode;
}

Spimcp::~Spimcp() {
    spi_bus_free(_host);
}

void Spimcp::spi_bus_init() {
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = _MISO;
    buscfg.mosi_io_num = _MOSI;
    buscfg.sclk_io_num = _SCLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    spi_bus_initialize(_host, &buscfg, SPI_DMA_CH_AUTO);
    

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = _clock_speed_hz;
    devcfg.mode = _mode;
    devcfg.spics_io_num = _CS;
    devcfg.queue_size = 1; 
    spi_bus_add_device(_host, &devcfg, &_spi_handle);
}

void Spimcp::mcp4132_write_register(uint8_t address, uint8_t value) {
    uint8_t buff[2];
    buff[0] = address;
    buff[1] = value;
   
    spi_transaction_t trans = {};
    trans.length = 16;
    trans.tx_buffer = buff; 
    spi_device_transmit(_spi_handle, &trans);
}

uint8_t Spimcp::mcp4132_read_register(uint8_t reg) {
    uint8_t tx_buff[2];
    uint8_t rx_buff[2];
    uint8_t rx_clean[2];
    
    tx_buff[0] = reg;

    tx_buff[1] = 0x00; // como es n, puede ser lo que sea

    spi_transaction_t t = {};
    t.length = 16; 
    t.tx_buffer = tx_buff;
    t.rx_buffer = rx_buff;
    spi_device_transmit(_spi_handle, &t);

    rx_clean[1] = rx_buff[1] | 0x7F;
    return rx_clean[1];

}
