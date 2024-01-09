#include <stdio.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../include/touch.h"
#include "../include/motors.h"

int main(void)
{
    printf("Waiting the EV3 brick online...\n");
    if (ev3_init() < 1)
        return 1;

    printf("*** ( EV3 ) Hello! ***\n");
    if (!setup_motors())
    {
        printf("Can't setup motors!");
        return 1;
    }
    // Raise the arm (just in case its down)
    move_motor(arm, 1, true);

    // Pick up the flag and wait for the arm to stop moving
    move_motor(arm, -1, true);

    // Move backwards (both wheels at the same time)
    move_motor_angle(leftWheel, -360, false);
    move_motor_angle(rightWheel, -360, false);
    /*printf("Waiting the EV3 brick online...\n");
    fflush(stdout);
    if (ev3_init() < 1)
        return (1);
    printf("*** ( EV3 ) Hello! ***\n");
    fflush(stdout);

    init_sensors();
    init_touch();
    printf("%d\n", get_touch());

    return 0;*/
}