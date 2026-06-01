#include "Ble.h"
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char* TAG = "Ble_Class";

// Inicializamos las variables estáticas
Ble* Ble::instance = nullptr;
uint16_t Ble::_conn_handle = BLE_HS_CONN_HANDLE_NONE;
uint16_t Ble::_tx_val_handle = 0;
bool Ble::_notify_enabled = false;
uint8_t Ble::_own_addr_type = 0;

// =======================================================
// DEFINICIÓN DE SERVICIOS Y CARACTERÍSTICAS (La "Pizarra")
// =======================================================

// UUIDs del Servicio UART de Nordic (Estándar de la industria)
static const ble_uuid128_t nus_service_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);

static const ble_uuid128_t nus_rx_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);

static const ble_uuid128_t nus_tx_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
                     0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

// Esta estructura le dice a NimBLE cómo armar la pizarra
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &nus_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {   // El post-it RX
                .uuid = &nus_rx_uuid.u,
                .access_cb = Ble::rx_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {   // El post-it TX
                .uuid = &nus_tx_uuid.u,
                .access_cb = Ble::tx_access_cb,
                // 🟢 CORRECCIÓN 2: En C++, 'flags' TIENE que ir antes que 'val_handle'
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &Ble::_tx_val_handle, 
            },
            { 0 } // Fin de las características
        },
    },
    { 0 } // Fin de los servicios
};


Ble::Ble(const char* deviceName) {
    _deviceName = deviceName;
    _lastCommand = '\0';
    _newDataAvailable = false;
    instance = this;
}

void Ble::init() {
    // 1. Inicializar la memoria flash (NimBLE guarda aquí los emparejamientos)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 2. Apagamos el Bluetooth Clásico para ahorrar RAM (Vamos a usar solo BLE)
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    // 3. Inicializamos NimBLE
    nimble_port_init();

    // 4. Inicializamos los servicios base de BLE (GAP = Anuncios, GATT = Datos)
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_gap_device_name_set(_deviceName); // Asignamos el nombre visible

    // 5. Conectamos los callbacks del sistema (Sincronización y Reinicio)
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;

    // 6. Cargamos nuestra "Pizarra" (Los servicios y UUIDs configurados arriba)
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

    // 7. Arrancamos NimBLE en una tarea dedicada de FreeRTOS
    nimble_port_freertos_init(host_task);
}

void Ble::send(const char* msg) {
    // Solo enviamos si hay un celular conectado Y si el celular activó las Notificaciones
    if (_conn_handle == BLE_HS_CONN_HANDLE_NONE || !_notify_enabled) {
        return;
    }

    // NimBLE usa "mbufs" (Memory Buffers) en lugar de strings normales por eficiencia
    struct os_mbuf *om = ble_hs_mbuf_from_flat(msg, strlen(msg));
    if (om != NULL) {
        // Hacemos "Ring" en la campana de notificación del celular con el nuevo dato
        ble_gatts_notify_custom(_conn_handle, _tx_val_handle, om);
    }
}

bool Ble::hasData() {
    return _newDataAvailable;
}

char Ble::getCommand() {
    _newDataAvailable = false;
    return _lastCommand;
}

// =======================================================
// CALLBACKS PRIVADOS (La magia detrás de escena)
// =======================================================

// Se ejecuta cuando el celular ESCRIBE algo en el ESP32
int Ble::rx_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint8_t data[64];
    uint16_t len = 0;

    // Extraemos el texto del buffer de NimBLE
    ble_hs_mbuf_to_flat(ctxt->om, data, sizeof(data) - 1, &len);
    data[len] = '\0';

    if (len > 0 && instance != nullptr) {
        instance->_lastCommand = (char)data[0];
        instance->_newDataAvailable = true; // Le avisamos al main que hay datos!
    }
    return 0;
}

// Callback de lectura (Obligatorio, aunque el celular solo escuche notificaciones)
int Ble::tx_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    return 0;
}

// Maneja la publicidad (Gritar al aire: "¡Existo, conéctate!")
void Ble::advertise(void) {
    struct ble_hs_adv_fields fields = {};
    struct ble_gap_adv_params adv_params = {};

    // Configuramos qué datos gritamos al aire (El UUID de nuestro servicio)
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128 = (ble_uuid128_t *)&nus_service_uuid;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // Arrancamos el transmisor de anuncios
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event_cb, NULL);
    
    ESP_LOGI(TAG, "BLE Transmitiendo (Advertising)...");
}

// Maneja conexiones, desconexiones y suscripciones a notificaciones
int Ble::gap_event_cb(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                _conn_handle = event->connect.conn_handle;
                ESP_LOGI(TAG, "¡Celular Conectado!");
            } else {
                advertise(); // Falló, volvemos a transmitir
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Celular Desconectado");
            _conn_handle = BLE_HS_CONN_HANDLE_NONE;
            _notify_enabled = false;
            advertise(); // Volvemos a transmitir para que otros se conecten
            break;

        case BLE_GAP_EVENT_SUBSCRIBE:
            // El celular presionó el botón de "Recibir Notificaciones" en la app
            if (event->subscribe.attr_handle == _tx_val_handle) {
                _notify_enabled = event->subscribe.cur_notify;
                ESP_LOGI(TAG, "Notificaciones activadas por el cliente");
            }
            break;
    }
    return 0;
}

void Ble::on_sync(void) {
    // El hardware de radio ya se sincronizó con la CPU, podemos arrancar
    ble_hs_id_infer_auto(0, &_own_addr_type);
    advertise();
}

void Ble::on_reset(int reason) {
    ESP_LOGE(TAG, "Reset BLE. Razón: %d", reason);
}

void Ble::host_task(void *param) {
    ESP_LOGI(TAG, "Tarea BLE corriendo...");
    nimble_port_run(); // Este bucle infinito procesa todos los eventos de radio
    nimble_port_freertos_deinit();
}