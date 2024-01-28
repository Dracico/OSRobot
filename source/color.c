#include "../include/color.h"

uint8_t port_color;

//Initialize the color sensors
int init_color(void)
{

    if (ev3_search_sensor(LEGO_EV3_COLOR, &port_color, 0))
    {
        printf("COLOR sensor is found on port %d!\n", port_color);
        fflush(stdout);

        // Set to
        set_sensor_mode(port_color, "COL-COLOR");
        printf("COLOR sensor is calibrated!\n");
        fflush(stdout);
        return 1;
    }
    else
    {
        printf("COLOR sensor is NOT found...\n");
        fflush(stdout);
        return 0;
    }
}

//Get the current color value from the sensor. We're looking at white = 6 as our main color
int get_color(void)
{
    int val;

    get_sensor_value(0, port_color, &val);

    return val;
}