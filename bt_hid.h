#ifndef _BT_HID_H_
#define _BT_HID_H_

#include <stdint.h>
#include <stdbool.h>

struct bt_hid_state {
    uint16_t buttons;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t l2;
    uint8_t r2;
    uint8_t hat;
    uint8_t pad;
};

void bt_hid_setup(void);
void bt_hid_update(void);
void bt_hid_get_latest(struct bt_hid_state* dst);
bool bt_hid_is_connected(void);
uint32_t bt_hid_ms_since_last_report(void);

#endif