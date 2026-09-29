# Setup and Hardware

This guide describes the uploaded source. The exact original ESP32 board, motor-driver model, power wiring and dependency versions are not recorded. The GPIO table is not a complete wiring schematic.

## Dependencies

- Arduino IDE with an ESP32/Bluepad32-compatible board package.
- Bluepad32 providing the gamepad APIs used in `bt_hid.cpp`.
- A VL53L0X library matching the `VL53L0X.h` API in `distance_sensors.cpp`.
- Arduino Wire library for I2C.

The motor code uses `ledcSetup` and `ledcAttachPin`. Verify that the selected package supports these APIs; compatibility with all current ESP32 packages is not established. Record versions after a successful build and hardware test rather than guessing the original versions.

## Local sketch preparation

1. Download or clone the repository.
2. Create a local folder named `esp32_BT__1_` and copy `esp32_BT__1_.ino` and all root `.cpp`/`.h` files into it. The primary sketch filename and folder basename must match for the usual Arduino sketch layout.
3. Open the sketch and select the actual board and port with a compatible package.
4. Install dependencies and compile; retain the version information and build output.
5. Check the physical wiring and test initially with wheels clear of the ground.
6. Upload and open Serial Monitor at **115200 baud**. Firmware prints controller, sensor and driving diagnostics.
7. Put the PS4 controller in pairing mode with **SHARE + PS**, as indicated in the startup output.

These are preparation steps, not a claim that a clean build or hardware test was performed for the September 2026 documentation update.

## GPIO assignments

| Signal | ESP32 GPIO |
| --- | --- |
| Left motor INA / INB / PWM | 25 / 26 / 27 |
| Right motor INA / INB / PWM | 16 / 17 / 18 |
| I2C SDA / SCL | 21 / 22 |
| Left / center / right sensor XSHUT | 32 / 33 / 15 |

Sensor addresses after initialization are `0x30`, `0x31` and `0x32`. Source configuration uses 100 kHz I2C and a requested 20,000 microsecond timing budget per sensor. Reads occur sequentially; this value is not the measured full-loop latency.

Motor PWM uses 5 kHz and 8-bit resolution. Confirm actual component supply requirements, grounding, motor polarity and board-specific pin constraints before use.

Read [known limitations](testing-and-limitations.md) before testing: the current code continues boost when all sensor readings are invalid. Verify behavior in a secured setup before controlled floor tests.
