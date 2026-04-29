#include <Arduino.h>
#include "motor_driver.h"
#include "pin_definitions.h"

static const int PWM_FREQ_HZ = 5000;
static const int PWM_RES_BITS = 8;
static const int PWM_CH_LEFT = 0;
static const int PWM_CH_RIGHT = 1;

static int clamp_motor(int val)
{
    if (val > 255) return 255;
    if (val < -255) return -255;
    return val;
}

void motor_driver_setup(void)
{
    Serial.println("motor driver setup");

    pinMode(PIN_LEFT_MOTOR_INA, OUTPUT);
    pinMode(PIN_LEFT_MOTOR_INB, OUTPUT);
    pinMode(PIN_RIGHT_MOTOR_INA, OUTPUT);
    pinMode(PIN_RIGHT_MOTOR_INB, OUTPUT);

    digitalWrite(PIN_LEFT_MOTOR_INA, LOW);
    digitalWrite(PIN_LEFT_MOTOR_INB, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_INA, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_INB, LOW);

    ledcSetup(PWM_CH_LEFT, PWM_FREQ_HZ, PWM_RES_BITS);
    ledcAttachPin(PIN_LEFT_MOTOR_PWM, PWM_CH_LEFT);
    ledcWrite(PWM_CH_LEFT, 0);

    ledcSetup(PWM_CH_RIGHT, PWM_FREQ_HZ, PWM_RES_BITS);
    ledcAttachPin(PIN_RIGHT_MOTOR_PWM, PWM_CH_RIGHT);
    ledcWrite(PWM_CH_RIGHT, 0);
}

void motor_driver_stop(void)
{
    digitalWrite(PIN_LEFT_MOTOR_INA, LOW);
    digitalWrite(PIN_LEFT_MOTOR_INB, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_INA, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_INB, LOW);

    ledcWrite(PWM_CH_LEFT, 0);
    ledcWrite(PWM_CH_RIGHT, 0);
}

void motor_driver_set_left(int val)
{
    val = clamp_motor(val);

    if (val > 0)
    {
        digitalWrite(PIN_LEFT_MOTOR_INA, LOW);
        digitalWrite(PIN_LEFT_MOTOR_INB, HIGH);
        ledcWrite(PWM_CH_LEFT, abs(val));
    }
    else if (val < 0)
    {
        digitalWrite(PIN_LEFT_MOTOR_INA, HIGH);
        digitalWrite(PIN_LEFT_MOTOR_INB, LOW);
        ledcWrite(PWM_CH_LEFT, abs(val));
    }
    else
    {
        digitalWrite(PIN_LEFT_MOTOR_INA, LOW);
        digitalWrite(PIN_LEFT_MOTOR_INB, LOW);
        ledcWrite(PWM_CH_LEFT, 0);
    }
}

void motor_driver_set_right(int val)
{
    val = clamp_motor(val);

    if (val > 0)
    {
        digitalWrite(PIN_RIGHT_MOTOR_INA, LOW);
        digitalWrite(PIN_RIGHT_MOTOR_INB, HIGH);
        ledcWrite(PWM_CH_RIGHT, abs(val));
    }
    else if (val < 0)
    {
        digitalWrite(PIN_RIGHT_MOTOR_INA, HIGH);
        digitalWrite(PIN_RIGHT_MOTOR_INB, LOW);
        ledcWrite(PWM_CH_RIGHT, abs(val));
    }
    else
    {
        digitalWrite(PIN_RIGHT_MOTOR_INA, LOW);
        digitalWrite(PIN_RIGHT_MOTOR_INB, LOW);
        ledcWrite(PWM_CH_RIGHT, 0);
    }
}