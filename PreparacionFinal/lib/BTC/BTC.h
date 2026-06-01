#ifndef __BTC_H__
#define __BTC_H__

#include <stdint.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

class Btc {
public:
    // El constructor recibe el nombre que verás en el celular al buscar el dispositivo
    Btc(const char* device_name);
    ~Btc();

    void init();                     // Inicializa la pila de Bluetooth completo
    void send(const char* msg);      // Envía texto al dispositivo conectado
    bool hasData();                  // Nos avisa si llegó un comando nuevo
    char getCommand();               // Extrae el comando recibido

private:
    const char* _deviceName;
    const char* _sppServerName;
    
    // Variables volátiles de comunicación interna
    volatile char _lastCommand;
    volatile bool _newDataAvailable;

    // Métodos estáticos obligatorios para que la librería de C de ESP-IDF los acepte
    static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
    static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    
    // Puntero estático para poder acceder a las variables de la clase desde los callbacks estáticos
    static Btc* instance; 
};

#endif