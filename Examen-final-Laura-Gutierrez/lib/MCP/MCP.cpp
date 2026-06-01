MCP::MCP() : _wiper(0), _spi(14, 12, 13) {
}

void Mcp::init() {
    _spi.spi_bus_init();
}

Mcp::~Mcp() {
}

void Mcp::mcp4132_set_wiper(uint8_t wiper) {
    _wiper = wiper;
    _spi.mcp4132_write_register(0x00, _wiper);
}

void Mcp::mcp4132_set_cutoff_frequency(uint8_t frequency) {
    uint8_t N = 0; 
    N = -8*(f*(8000000*3.14+3)-4000000)/(25*frequency);
    _spi.mcp4132_write_register(0x01, N);
}