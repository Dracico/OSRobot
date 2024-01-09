#ifndef MOTORS_H
#define MOTORS_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_port.h"
#include "../ev3dev-c/source/ev3/ev3_tacho.h"

extern uint8_t leftWheel;
extern uint8_t rightWheel;
extern uint8_t arm;

bool setup_motors(void);

void move_motor(uint8_t sn, int direction, bool blocking);
void move_motor_angle(uint8_t sn, int degrees, bool blocking);
void wait_motor_stop();

#endif