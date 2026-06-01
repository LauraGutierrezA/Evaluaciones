#include "BTC.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>

// Inicializamos el puntero estático de la instancia en nulo
Btc* Btc::instance = nullptr;
static const char* TAG = "Btc_Class";
static uint32_t spp_handle = 0; // Identificador único de la conexión activa

Btc::Btc(const char* device_name) {
    _deviceName = device_name;
    _sppServerName = "SPP_SERVER_CLASS";
    _lastCommand = '\0';
    _newDataAvailable = false;
    instance = this; // Guardamos la dirección de ESTE objeto para los callbacks
}

Btc::~Btc() {
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
}

void Btc::init() {
    esp_err_t ret;

    // 1. Inicializar la memoria NVS (Memoria no volátil). 
    // Bluetooth guarda aquí las llaves y credenciales de los celulares vinculados anteriormente.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 2. Liberar la memoria RAM asignada a BLE (Bluetooth Low Energy)
    // Como vamos a usar Bluetooth Clásico (SPP), recuperamos esa RAM descartando BLE.
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    // 3. Configurar e Inicializar el controlador físico del Hardware de Radio Bluetooth
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);

    // 4. Inicializar "Bluedroid" (La pila de software que maneja los protocolos lógicos)
    esp_bluedroid_init();
    esp_bluedroid_enable();

    // 5. Registrar las funciones Callbacks en el sistema operativo
    // Le decimos al ESP32 a qué funciones llamar cuando pase algo en el radio (GAP) o en los datos (SPP)
    esp_bt_gap_register_callback(gap_callback);
    esp_spp_register_callback(spp_callback);

    // 6. Configurar el modo de operación del puerto serie Bluetooth (SPP)
    esp_spp_cfg_t spp_cfg = {
        .mode = ESP_SPP_MODE_CB, // Modo Callback (asincrónico basado en eventos)
        .enable_l2cap_ertm = true,
        .tx_buffer_size = 0
    };
    esp_spp_enhanced_init(&spp_cfg);
}

void Btc::send(const char* msg) {
    // Si spp_handle no es 0, significa que hay un canal de comunicación abierto con un cliente
    if (spp_handle != 0) {
        esp_spp_write(spp_handle, strlen(msg), (uint8_t *)msg);
    }
}

bool Btc::hasData() {
    return _newDataAvailable;
}

char Btc::getCommand() {
    _newDataAvailable = false; // Bajamos la bandera automáticamente al leer el dato
    return _lastCommand;
}

// ==========================================
// CALLBACKS (El conmutador de eventos del ESP32)
// ==========================================

void Btc::gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    if (event == ESP_BT_GAP_AUTH_CMPL_EVT) {
        ESP_LOGI(TAG, "Autenticación Bluetooth completada de forma segura");
    }
}

void Btc::spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    // Como las funciones son estáticas, usamos "instance->" para poder modificar las variables privadas del objeto
    if (instance == nullptr) return;

    switch (event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(TAG, "SPP Inicializado. Configurando visibilidad...");
            // Asignamos el nombre comercial del dispositivo
            esp_bt_gap_set_device_name(instance->_deviceName);
            // Ponemos el ESP32 en modo "Visible" para que cualquier celular lo encuentre al buscar Bluetooth
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            // Arrancamos el servidor del puerto serie
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, instance->_sppServerName);
            break;

        case ESP_SPP_SRV_OPEN_EVT:
            // ¡Se conectó un cliente! Guardamos su número de handle para poder enviarle datos después
            spp_handle = param->srv_open.handle;
            ESP_LOGI(TAG, "¡Celular/Computador conectado por Bluetooth!");
            instance->send("Conectado exitosamente al ESP32\r\n");
            break;

        case ESP_SPP_CLOSE_EVT:
            // Se desconectó el cliente. Ponemos el handle en cero para bloquear envíos accidentales
            spp_handle = 0;
            ESP_LOGI(TAG, "Dispositivo Bluetooth desconectado");
            break;

        case ESP_SPP_DATA_IND_EVT:
            // ¡Llegaron datos desde el aire!
            if (param->data_ind.len > 0) {
                // Guardamos el primer carácter recibido en la variable privada de nuestra clase
                instance->_lastCommand = (char)param->data_ind.data[0];
                instance->_newDataAvailable = true; // Encendemos la bandera para avisarle al main
            }
            break;

        default:
            break;
    }
}