#ifndef SONAR_H
#define SONAR_H

#include <stdio.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_sensor.h"

extern uint8_t port_sonar;

int init_sonar(void);
int get_sonar(void);

#endif