#include "I2CR.h"

I2cr::I2cr(i2c_port_t I2C_PORT, uint8_t I2C_SDA_PIN, uint8_t I2C_SCL_PIN, uint32_t I2C_FREQ_HZ, uint8_t SLAVE_ADDR){
    _i2cPort = I2C_PORT;
    _sdaPin = I2C_SDA_PIN;
    _sclPin = I2C_SCL_PIN;
    _clkSpeed = I2C_FREQ_HZ;
    _slaveAddr = SLAVE_ADDR;
}

void I2cr::init(void) {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = _sdaPin;
    conf.scl_io_num = _sclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = _clkSpeed;

    i2c_param_config(_i2cPort, &conf);
    i2c_driver_install(_i2cPort, conf.mode, 0, 0, 0);
}

I2cr::~I2cr() {
    i2c_driver_delete(_i2cPort);
}

uint8_t I2cr::decimal_a_bcd(uint8_t valor) {
    return ((valor / 10) << 4) | (valor % 10);
}
uint8_t I2cr::bcd_a_decimal(uint8_t valor) {
    return ((valor >> 4) * 10) + (valor & 0x0F);
}

esp_err_t I2cr::readTime(RtcTime *time) {
    uint8_t datos[7];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Secuencia de lectura: Write (registro) + Restart + Read (datos)
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_slaveAddr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_slaveAddr << 1) | I2C_MASTER_READ, true);
    
    // Leemos 6 bytes con ACK, el último con NACK
    i2c_master_read(cmd, datos, 6, I2C_MASTER_ACK); //lee los promeros 6 bytes con ACK
    i2c_master_read_byte(cmd, &datos[6], I2C_MASTER_NACK); //lee el último byte con NACK, por eso es read byte y no read, porque solo lee un byte
    
    i2c_master_stop(cmd);
    
    // Ejecutamos el comando y obtenemos el resultado
    esp_err_t resultado = i2c_master_cmd_begin(_i2cPort, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (resultado == ESP_OK) {
        // Conversión de BCD a Decimal con sus máscaras necesarias
        time->seconds        = bcd_a_decimal(datos[0] & 0x7F);
        time->minutes        = bcd_a_decimal(datos[1] & 0x7F);
        time->hours       = bcd_a_decimal(datos[2] & 0x3F);
        time->dayOfWeek  = bcd_a_decimal(datos[3] & 0x07);
        time->dayOfMonth = bcd_a_decimal(datos[4] & 0x3F);
        time->month      = bcd_a_decimal(datos[5] & 0x1F);
        time->year       = bcd_a_decimal(datos[6]);
    }
    
    return resultado;
};

esp_err_t I2cr::writeTime(RtcTime time) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_slaveAddr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    
    // Usamos los campos de la estructura
    i2c_master_write_byte(cmd, decimal_a_bcd(time.seconds), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.minutes), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.hours), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.dayOfWeek), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.dayOfMonth), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.month), true);
    i2c_master_write_byte(cmd, decimal_a_bcd(time.year), true);
    
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(_i2cPort, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}
