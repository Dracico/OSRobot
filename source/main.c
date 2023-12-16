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

    move_motor(leftWheel, 1);
    move_motor(rightWheel, 1);
    // move_motor(arm, -1);
    // move_motor(arm, 1);
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