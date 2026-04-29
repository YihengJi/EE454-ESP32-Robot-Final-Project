#include <Arduino.h>
#include "motor_driver.h"
#include "bt_hid.h"
#include "distance_sensors.h"

#define COMMAND_TIMEOUT_MS 300

// L2 trigger threshold
#define BOOST_L2_THRESHOLD 40

// Manual override deadzone
#define STICK_OVERRIDE_DEADZONE 15

// Speed settings
#define MANUAL_MAX_SPEED 190
#define BOOST_FULL_SPEED 255

// Stop threshold: average distance < 0.7 m
#define BOOST_STOP_AVG_THRESHOLD 0.70f

enum SystemStates
{
    SystemState_DISCONNECTED,
    SystemState_CONNECTED,
};

static SystemStates next_system_state = SystemState_DISCONNECTED;
static SystemStates system_state = SystemState_DISCONNECTED;
static SystemStates prev_system_state = SystemState_DISCONNECTED;

static struct bt_hid_state controller_state = {0};
static distance_data_t dist_data = {9.99f, 9.99f, 9.99f};

static int last_left_cmd = 0;
static int last_right_cmd = 0;

static bool boost_mode = false;

static int clamp_int(int val, int min_val, int max_val)
{
    if (val > max_val) return max_val;
    if (val < min_val) return min_val;
    return val;
}

static bool stick_moved_for_override(const bt_hid_state* state)
{
    if (abs((int)state->lx - 128) > STICK_OVERRIDE_DEADZONE) return true;
    if (abs((int)state->ly - 128) > STICK_OVERRIDE_DEADZONE) return true;
    if (abs((int)state->rx - 128) > STICK_OVERRIDE_DEADZONE) return true;
    if (abs((int)state->ry - 128) > STICK_OVERRIDE_DEADZONE) return true;
    return false;
}

static void compute_manual_drive(const bt_hid_state* state,
                                 int* left_cmd,
                                 int* right_cmd)
{
    int forward_speed = (256 - state->ly) - 128;
    int swerve_right_rate = (state->rx - 128);

    int left_output = forward_speed;
    int right_output = forward_speed;

    if (forward_speed > 20)
    {
        if (swerve_right_rate > 0)
        {
            right_output -= swerve_right_rate;
            right_output = clamp_int(right_output, 0, 128);
        }
        else if (swerve_right_rate < 0)
        {
            left_output += swerve_right_rate;
            left_output = clamp_int(left_output, 0, 128);
        }
    }
    else if (forward_speed < -20)
    {
        if (swerve_right_rate > 0)
        {
            right_output += swerve_right_rate;
            right_output = clamp_int(right_output, -127, 0);
        }
        else if (swerve_right_rate < 0)
        {
            left_output -= swerve_right_rate;
            left_output = clamp_int(left_output, -127, 0);
        }
    }
    else
    {
        if (swerve_right_rate >= 20 || swerve_right_rate <= -20)
        {
            left_output = swerve_right_rate;
            right_output = -swerve_right_rate;
        }
        else
        {
            left_output = 0;
            right_output = 0;
        }
    }

    // 手动模式映射到 190 上限
    left_output = left_output * MANUAL_MAX_SPEED / 128;
    right_output = right_output * MANUAL_MAX_SPEED / 128;

    left_output = clamp_int(left_output, -MANUAL_MAX_SPEED, MANUAL_MAX_SPEED);
    right_output = clamp_int(right_output, -MANUAL_MAX_SPEED, MANUAL_MAX_SPEED);

    if (left_cmd)  *left_cmd = left_output;
    if (right_cmd) *right_cmd = right_output;
}

static float compute_valid_average_distance(const distance_data_t* d)
{
    float sum = 0.0f;
    int count = 0;

    if (d->left_m >= 0.0f) {
        sum += d->left_m;
        count++;
    }
    if (d->center_m >= 0.0f) {
        sum += d->center_m;
        count++;
    }
    if (d->right_m >= 0.0f) {
        sum += d->right_m;
        count++;
    }

    if (count == 0) {
        return -1.0f;
    }

    return sum / count;
}

static void apply_boost_average_stop_logic(int* left_cmd, int* right_cmd, const distance_data_t* d)
{
    float avg_dist = compute_valid_average_distance(d);

    // Default boost behavior
    *left_cmd = BOOST_FULL_SPEED;
    *right_cmd = BOOST_FULL_SPEED;

    // If no valid sensor data, keep moving
    if (avg_dist < 0.0f) {
        Serial.println("BOOST: no valid sensor data");
        return;
    }

    // Stop if average distance is below threshold
    if (avg_dist < BOOST_STOP_AVG_THRESHOLD)
    {
        *left_cmd = 0;
        *right_cmd = 0;
        Serial.println("BOOST: AVG DISTANCE STOP");
        return;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("ESP32 final control start");

    motor_driver_setup();
    bt_hid_setup();

    bool sensors_ok = distance_sensors_setup();
    Serial.print("distance sensors setup: ");
    Serial.println(sensors_ok ? "OK" : "FAIL");

    next_system_state = SystemState_DISCONNECTED;
    system_state = SystemState_DISCONNECTED;
    prev_system_state = SystemState_DISCONNECTED;
    boost_mode = false;
}

void loop()
{
    bt_hid_update();

    bool system_state_entered = (system_state != prev_system_state);
    bt_hid_get_latest(&controller_state);

    switch (system_state)
    {
        case SystemState_DISCONNECTED:
        {
            if (system_state_entered)
            {
                Serial.println("SystemState_DISCONNECTED");
                motor_driver_stop();
                last_left_cmd = 0;
                last_right_cmd = 0;
                boost_mode = false;
            }

            if (bt_hid_is_connected())
            {
                next_system_state = SystemState_CONNECTED;
            }
            break;
        }

        case SystemState_CONNECTED:
        {
            if (system_state_entered)
            {
                Serial.println("SystemState_CONNECTED");
                boost_mode = false;
            }

            if (!bt_hid_is_connected())
            {
                next_system_state = SystemState_DISCONNECTED;
                break;
            }

            uint32_t ms_since_last_report = bt_hid_ms_since_last_report();
            if (ms_since_last_report > COMMAND_TIMEOUT_MS)
            {
                motor_driver_stop();
                last_left_cmd = 0;
                last_right_cmd = 0;
                boost_mode = false;
                break;
            }

            if (boost_mode && stick_moved_for_override(&controller_state))
            {
                boost_mode = false;
                Serial.println("Boost cancelled by manual stick input");
            }

            if (!boost_mode &&
                controller_state.l2 > BOOST_L2_THRESHOLD &&
                !stick_moved_for_override(&controller_state))
            {
                boost_mode = true;
                Serial.println("Boost mode entered");
            }

            if (boost_mode)
            {
                if (controller_state.l2 <= BOOST_L2_THRESHOLD)
                {
                    boost_mode = false;
                    Serial.println("Boost mode released");
                    compute_manual_drive(&controller_state, &last_left_cmd, &last_right_cmd);
                }
                else
                {
                    distance_sensors_update();
                    distance_sensors_get_latest(&dist_data);
                    apply_boost_average_stop_logic(&last_left_cmd, &last_right_cmd, &dist_data);
                }
            }
            else
            {
                compute_manual_drive(&controller_state, &last_left_cmd, &last_right_cmd);

                dist_data.left_m = 9.99f;
                dist_data.center_m = 9.99f;
                dist_data.right_m = 9.99f;
            }

            motor_driver_set_left(last_left_cmd);
            motor_driver_set_right(last_right_cmd);

            static uint32_t last_print_ms = 0;
            if (millis() - last_print_ms > 200)
            {
                last_print_ms = millis();

                float avg_dist = compute_valid_average_distance(&dist_data);

                Serial.print("mode=");
                Serial.print(boost_mode ? "BOOST" : "MANUAL");

                Serial.print(" | L2=");
                Serial.print(controller_state.l2);

                Serial.print(" | LY=");
                Serial.print(controller_state.ly);
                Serial.print(" RX=");
                Serial.print(controller_state.rx);

                Serial.print(" | DL=");
                Serial.print(dist_data.left_m, 2);
                Serial.print(" DC=");
                Serial.print(dist_data.center_m, 2);
                Serial.print(" DR=");
                Serial.print(dist_data.right_m, 2);

                Serial.print(" | AVG=");
                Serial.print(avg_dist, 2);

                Serial.print(" | ML=");
                Serial.print(last_left_cmd);
                Serial.print(" MR=");
                Serial.println(last_right_cmd);
            }
            break;
        }

        default:
        {
            motor_driver_stop();
            next_system_state = SystemState_DISCONNECTED;
            boost_mode = false;
            break;
        }
    }

    prev_system_state = system_state;
    system_state = next_system_state;

    delay(5);
}