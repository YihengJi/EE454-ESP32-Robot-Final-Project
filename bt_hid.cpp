#include <Arduino.h>
#include <Bluepad32.h>
#include <string.h>
#include "bt_hid.h"

static GamepadPtr myGamepad = nullptr;

static bt_hid_state g_latest = {
    .buttons = 0,
    .lx = 128,
    .ly = 128,
    .rx = 128,
    .ry = 128,
    .l2 = 0,
    .r2 = 0,
    .hat = 8,
    .pad = 0,
};

static bool g_connected = false;
static uint32_t g_last_report_ms = 0;

static uint8_t clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

static uint8_t axis_to_u8(int v) {
    int mapped = v + 512;
    mapped = mapped / 4;
    return clamp_u8(mapped);
}

static uint8_t trigger_to_u8(int v) {
    v = v / 4;
    return clamp_u8(v);
}

static uint8_t dpad_to_hat(GamepadPtr gp) {
    bool up = gp->dpad() & DPAD_UP;
    bool down = gp->dpad() & DPAD_DOWN;
    bool left = gp->dpad() & DPAD_LEFT;
    bool right = gp->dpad() & DPAD_RIGHT;

    if (up && right) return 1;
    if (right && down) return 3;
    if (down && left) return 5;
    if (left && up) return 7;
    if (up) return 0;
    if (right) return 2;
    if (down) return 4;
    if (left) return 6;
    return 8;
}

static uint16_t buttons_to_u16(GamepadPtr gp) {
    uint16_t b = 0;
    uint16_t bp = gp->buttons();

    if (bp & BUTTON_X)           b |= (1 << 0);
    if (bp & BUTTON_A)           b |= (1 << 1);
    if (bp & BUTTON_B)           b |= (1 << 2);
    if (bp & BUTTON_Y)           b |= (1 << 3);
    if (bp & BUTTON_SHOULDER_L)  b |= (1 << 4);
    if (bp & BUTTON_SHOULDER_R)  b |= (1 << 5);
    if (bp & BUTTON_THUMB_L)     b |= (1 << 10);
    if (bp & BUTTON_THUMB_R)     b |= (1 << 11);

    uint16_t misc = gp->miscButtons();
    if (misc & MISC_BUTTON_SELECT) b |= (1 << 8);
    if (misc & MISC_BUTTON_START)  b |= (1 << 9);
    if (misc & MISC_BUTTON_SYSTEM) b |= (1 << 12);

    return b;
}

static void reset_state() {
    g_latest.buttons = 0;
    g_latest.lx = 128;
    g_latest.ly = 128;
    g_latest.rx = 128;
    g_latest.ry = 128;
    g_latest.l2 = 0;
    g_latest.r2 = 0;
    g_latest.hat = 8;
    g_latest.pad = 0;
}

static void onConnectedGamepad(GamepadPtr gp) {
    myGamepad = gp;
    g_connected = true;
    g_last_report_ms = millis();
    Serial.println("Gamepad connected");
}

static void onDisconnectedGamepad(GamepadPtr gp) {
    if (gp == myGamepad) {
        myGamepad = nullptr;
        g_connected = false;
        reset_state();
        g_last_report_ms = millis();
        Serial.println("Gamepad disconnected");
    }
}

void bt_hid_setup(void) {
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("Bluepad32 local addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    Serial.println("Put controller in pairing mode: SHARE + PS");
    g_last_report_ms = millis();
}

void bt_hid_update(void) {
    BP32.update();

    if (!myGamepad || !myGamepad->isConnected()) {
        return;
    }

    g_latest.buttons = buttons_to_u16(myGamepad);
    g_latest.lx = axis_to_u8(myGamepad->axisX());
    g_latest.ly = axis_to_u8(myGamepad->axisY());
    g_latest.rx = axis_to_u8(myGamepad->axisRX());
    g_latest.ry = axis_to_u8(myGamepad->axisRY());
    g_latest.l2 = trigger_to_u8(myGamepad->brake());
    g_latest.r2 = trigger_to_u8(myGamepad->throttle());
    g_latest.hat = dpad_to_hat(myGamepad);
    g_latest.pad = 0;

    g_last_report_ms = millis();
}

void bt_hid_get_latest(struct bt_hid_state* dst) {
    if (!dst) return;
    memcpy(dst, &g_latest, sizeof(*dst));
}

bool bt_hid_is_connected(void) {
    return g_connected && myGamepad && myGamepad->isConnected();
}

uint32_t bt_hid_ms_since_last_report(void) {
    return millis() - g_last_report_ms;
}