#include <stdio.h>
#include "../include/sensors.h"
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../ev3dev-c/source/ev3/ev3_sensor.h"

static bool _check_pressed(uint8_t sn)
{
    int val;

    if (sn == SENSOR__NONE_)
    {
        return (ev3_read_keys((uint8_t *)&val) && (val & EV3_KEY_UP));
    }
    return (get_sensor_value(0, sn, &val) && (val != 0));
}

int main(void)
{
    uint8_t sn_touch;

    printf("Waiting the EV3 brick online...\n");
    fflush(stdout);
    if (ev3_init() < 1)
        return (1);

    printf("*** ( EV3 ) Hello! ***\n");
    fflush(stdout);
    ev3_sensor_init();

    printf("Found sensors:\n");
    fflush(stdout);

    if (ev3_search_sensor(LEGO_EV3_TOUCH, &sn_touch, 0))
    {
        printf("TOUCH sensor is found on port %d!\n", sn_touch);
        fflush(stdout);
        while (1)
        {
            if (_check_pressed(sn_touch))
            {
                printf("Pressed!\n");
                fflush(stdout);
            }
        }
    }
    else
    {
        printf("TOUCH sensor is NOT found...\n");
    }

    return 0;
}