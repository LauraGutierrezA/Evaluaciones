#ifndef __BLE_H__
#define __BLE_H__

#include <stdint.h>
#include <stdbool.h>

struct ble_gap_event;
struct ble_gatt_access_ctxt;

class Ble {
public:
    Ble(const char* deviceName);
    void init();
    void send(const char* msg);
    bool hasData();
    char getCommand();

    // 🟢 CORRECCIÓN 1: Movemos esto a la zona pública para que NimBLE pueda acceder
    static uint16_t _tx_val_handle;
    static int rx_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
    static int tx_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

private:
    const char* _deviceName;
    volatile char _lastCommand;
    volatile bool _newDataAvailable;

    static uint16_t _conn_handle;
    static bool _notify_enabled;
    static uint8_t _own_addr_type;
    static Ble* instance;

    static int gap_event_cb(struct ble_gap_event *event, void *arg);
    static void on_sync(void);
    static void on_reset(int reason);
    static void advertise(void);
    static void host_task(void *param);
};

#endif