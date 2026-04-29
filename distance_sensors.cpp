#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>
#include "pin_definitions.h"
#include "distance_sensors.h"

static VL53L0X sensor_left;
static VL53L0X sensor_center;
static VL53L0X sensor_right;

static distance_data_t g_dist = {9.99f, 9.99f, 9.99f};

static const uint8_t ADDR_LEFT   = 0x30;
static const uint8_t ADDR_CENTER = 0x31;
static const uint8_t ADDR_RIGHT  = 0x32;

static float mm_to_m(uint16_t mm) {
    return ((float)mm) / 1000.0f;
}

static bool init_one_sensor(VL53L0X& sensor, int xshut_pin, uint8_t new_addr) {
    digitalWrite(xshut_pin, HIGH);
    delay(50);

    sensor.setTimeout(100);

    if (!sensor.init()) {
        return false;
    }

    delay(20);
    sensor.setAddress(new_addr);
    delay(20);

    sensor.setMeasurementTimingBudget(20000);
    return true;
}

bool distance_sensors_setup(void) {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);

    pinMode(PIN_XSHUT_LEFT, OUTPUT);
    pinMode(PIN_XSHUT_CENTER, OUTPUT);
    pinMode(PIN_XSHUT_RIGHT, OUTPUT);

    digitalWrite(PIN_XSHUT_LEFT, LOW);
    digitalWrite(PIN_XSHUT_CENTER, LOW);
    digitalWrite(PIN_XSHUT_RIGHT, LOW);
    delay(100);

    bool ok_left = init_one_sensor(sensor_left, PIN_XSHUT_LEFT, ADDR_LEFT);
    delay(50);

    bool ok_center = init_one_sensor(sensor_center, PIN_XSHUT_CENTER, ADDR_CENTER);
    delay(50);

    bool ok_right = init_one_sensor(sensor_right, PIN_XSHUT_RIGHT, ADDR_RIGHT);
    delay(50);

    return ok_left && ok_center && ok_right;
}

void distance_sensors_update(void) {
    uint16_t left_mm = sensor_left.readRangeSingleMillimeters();
    bool left_bad = sensor_left.timeoutOccurred() || left_mm == 65535 || left_mm > 8000;

    uint16_t center_mm = sensor_center.readRangeSingleMillimeters();
    bool center_bad = sensor_center.timeoutOccurred() || center_mm == 65535 || center_mm > 8000;

    uint16_t right_mm = sensor_right.readRangeSingleMillimeters();
    bool right_bad = sensor_right.timeoutOccurred() || right_mm == 65535 || right_mm > 8000;

    g_dist.left_m = left_bad ? -1.0f : mm_to_m(left_mm);
    g_dist.center_m = center_bad ? -1.0f : mm_to_m(center_mm);
    g_dist.right_m = right_bad ? -1.0f : mm_to_m(right_mm);
}

void distance_sensors_get_latest(distance_data_t* dst) {
    if (!dst) return;
    *dst = g_dist;
}