#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_sensor.h"

extern uint8_t port_color;

int init_color(void);
int get_color(void);

#endif