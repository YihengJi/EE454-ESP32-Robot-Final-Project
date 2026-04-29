#ifndef _DISTANCE_SENSORS_H_
#define _DISTANCE_SENSORS_H_

#include <stdbool.h>

struct distance_data_t {
    float left_m;
    float center_m;
    float right_m;
};

bool distance_sensors_setup(void);
void distance_sensors_update(void);
void distance_sensors_get_latest(distance_data_t* dst);

#endif