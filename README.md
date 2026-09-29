# ESP32 Robot Control and Obstacle-Based Stopping

An undergraduate team project at the University of Evansville for **EE-454 Microcontroller Applications, Spring 2026**. The ESP32 implementation extends an earlier CRFC football robot with PS4 manual driving, L2-triggered boost, and distance-based stopping during boost.

## Context and contributions

The platform originated in the Collegiate Robotic Football Conference (CRFC) project, with faculty guidance from **Professors Yishu Bai and John MacDonald**. The EE-454 extension was a team project.

**Yiheng Ji's contributions** included adapting and reorganizing control software for the ESP32 migration, reconfiguring wiring, installing and integrating three VL53L0X sensors, debugging individual and combined readings, and conducting driving and stopping tests. The work builds on an existing robot platform and third-party libraries; it is not a firmware stack written entirely from scratch.

The [earlier Pico W build logs and photographs](https://github.com/YihengJi/CRFC-Football-robot-notes) are maintained separately. The later vision/AI quarterback senior design is a separate project and is not implemented here.

## Implemented behavior

| Feature | Behavior in the uploaded source |
| --- | --- |
| Manual driving | Left-stick vertical input controls forward/reverse; right-stick horizontal input controls turning. |
| Boost | L2 above the configured threshold with sticks centered commands forward PWM 255; manual PWM is limited to 190. These are duty commands, not measured speeds. |
| Manual override | Stick movement cancels boost; releasing L2 returns to manual control. |
| Distance-based stop | During boost, the arithmetic mean of valid sensor readings below 0.70 m sets motor commands to zero. |
| Sensor integration | Three VL53L0X sensors share I2C using separate XSHUT pins and assigned addresses. |
| Controller disconnect | A motor-stop path is included in the main state machine. |

## Results and evidence

- Original project tests reported functioning manual driving, boost, and distance-triggered stopping.
- Light obstacle contact was observed with a 0.5 m stopping threshold; the threshold was increased to 0.7 m. This was an empirical adjustment, not a measured clearance guarantee.
- Source code and [test notes](testing-and-limitations.md) distinguish implemented behavior from reported observations and future work.
- No quantitative stopping-distance dataset or fault-injection validation is included. Speed-adaptive braking remains a future investigation.

## Read before running

This is a course prototype, not a validated autonomous navigation or collision-avoidance system. The current code has important limitations:

1. **All invalid sensor readings leave boost commanded forward.** Sensor failure does not cause a fail-closed stop.
2. **The controller timestamp refreshes on every connected update loop**, without confirming receipt of a new input report. The 300 ms check therefore does not establish detection of stale commands while connected.
3. The stopping rule uses the **average**, not the nearest obstacle distance, and applies **only during boost**.
4. Zero PWM is not an instantaneous physical stop. Sequential sensor reads can delay further controller processing.

Use a secured setup with wheels clear of the ground for initial tests and an accessible power cutoff. These limitations are documented without changing the original course firmware.

## Repository guide

| File | Purpose |
| --- | --- |
| [`esp32_BT__1_.ino`](esp32_BT__1_.ino) | Main state machine, manual/boost logic and stopping threshold. |
| [`bt_hid.cpp`](bt_hid.cpp), [`bt_hid.h`](bt_hid.h) | Bluepad32 controller interface and input state. |
| [`motor_driver.cpp`](motor_driver.cpp), [`motor_driver.h`](motor_driver.h) | Direction and PWM output. |
| [`distance_sensors.cpp`](distance_sensors.cpp), [`distance_sensors.h`](distance_sensors.h) | Sensor initialization, addressing and reads. |
| [`pin_definitions.h`](pin_definitions.h) | GPIO assignments. |
| [Setup and hardware](setup.md) | Dependencies, pin mapping and build preparation. |
| [Testing and limitations](testing-and-limitations.md) | Evidence, caveats and proposed validation. |

## Dependencies and future work

The source uses Arduino APIs, Bluepad32, Wire, and the `VL53L0X.h` library interface. Original package versions were not recorded; a clean-environment build has not been verified during this documentation update.

Next steps include correcting report-freshness tracking, defining and testing sensor-failure behavior, measuring sensing latency and stopping distance, and comparing fixed-threshold with speed-aware stopping methods.

Documentation reviewed September 2026. Firmware remains the uploaded April 2026 course version.
