# EE454 ESP32 Robot Final Project

This repository contains the source code for our EE-454 final project robot.

The project is an ESP32-based mobile robot rebuilt from an earlier Pico W robot platform. The final system supports stable PS4 manual control, L2 boost mode, and distance-sensor-based automatic stopping.

## Main Completed Functions

- Stable PS4 controller connection on ESP32
- Forward, backward, left, and right movement
- L2 boost mode
- Three VL53L0X distance sensors
- Automatic stop when the average valid sensor distance is below 0.7 m during boost mode

## Project Motivation

We changed the main controller from Pico W to ESP32 in order to improve connection stability, debugging convenience, code modification workflow, and future expandability.

Compared with the previous Pico W-based setup, the ESP32 platform provided:
- more stable controller integration
- easier reflashing and repeated testing
- direct feedback through Serial Monitor
- stronger library support for controller and sensor expansion

## Code File Overview

### Main Control Logic
- `esp32_BT_1_.ino`  
  Main program of the robot. Handles manual mode, boost mode, controller logic, and automatic stopping behavior.

### Controller Interface
- `bt_hid.h`
- `bt_hid.cpp`  
  Handles PS4 controller communication on ESP32 and provides controller input data to the main program.

### Motor Driver Control
- `motor_driver.h`
- `motor_driver.cpp`  
  Controls motor direction and PWM output for the left and right wheels.

### Pin Definitions
- `pin_definitions.h`  
  Contains GPIO assignments for motors, sensors, and other hardware connections.

### Distance Sensor Logic
- `distance_sensors.h`
- `distance_sensors.cpp`  
  Initializes and reads the three VL53L0X distance sensors used for boost-mode stopping.

## Hardware Summary

The robot hardware includes:
- ESP32 development board and expansion board
- Motor driver
- Left and right drive motors
- PS4 controller for wireless manual control
- Three VL53L0X distance sensors
- Battery, power distribution, and internal wiring integrated into the chassis

## Final Behavior

In manual mode, the robot can be driven normally using the PS4 controller.

When L2 is pressed:
- the robot enters boost mode
- the robot moves forward at high speed
- the three distance sensors are enabled
- the robot stops automatically if the average valid sensor distance becomes smaller than 0.7 m

Manual stick input always overrides automatic behavior.

## Notes

This repository is intended to present the final ESP32-based version of the EE-454 robot project.
