#include "../include/sonar.h"

uint8_t port_sonar;

int init_sonar(void)
{

    if (ev3_search_sensor(LEGO_EV3_US, &port_sonar, 0))
    {
        printf("SONAR sensor is found on port %d!\n", port_sonar);
        fflush(stdout);

        // Set to
        set_sensor_mode(port_sonar, "US-DIST-CM");
        printf("SONAR sensor is calibrated!\n");
        fflush(stdout);
        return 1;
    }
    else
    {
        printf("SONAR sensor is NOT found...\n");
        fflush(stdout);
        return 0;
    }
}

int get_sonar(void)
{
    int val;

    get_sensor_value(0, port_sonar, &val);

    return val;
}