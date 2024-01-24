#ifndef GYRO_H
#define GYRO_H

#include <stdio.h>
#include <unistd.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_sensor.h"

extern uint8_t port_touch;

int init_gyro(void);
int get_gyro(void);

#endif