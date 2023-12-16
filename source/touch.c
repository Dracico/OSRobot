#include "../include/touch.h"

uint8_t port_touch;

int init_sensors(void)
{
    return ev3_sensor_init();
}

int init_touch(void)
{

    if (ev3_search_sensor(LEGO_EV3_TOUCH, &port_touch, 0))
    {
        printf("TOUCH sensor is found on port %d!\n", port_touch);
        fflush(stdout);
        return 1;
    }
    else
    {
        printf("TOUCH sensor is NOT found...\n");
        fflush(stdout);
        return 0;
    }
}

bool get_touch(void)
{
    int val;
    return (get_sensor_value(0, port_touch, &val) && (val != 0));
}