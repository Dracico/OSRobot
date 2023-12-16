#ifndef SENSORS_H
#define SENSORS_H

#include <stdio.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_sensor.h"

extern uint8_t port_touch;

int init_sensors(void);
int init_touch(void);
bool get_touch(void);

#endif