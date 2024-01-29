#include "../include/gyro.h"

uint8_t port_gyro;

//Initialize the gyro sensor. Some accuracy problems are sometimes detected
// May need to reboot the robot to fix them
int init_gyro(void)
{

    if (ev3_search_sensor(LEGO_EV3_GYRO, &port_gyro, 0))
    {
        printf("GYRO sensor is found on port %d!\n", port_gyro);
        fflush(stdout);
        // In order to set the angle back to 0 at the start, we need to switch between twp different modes (just a bug?)
        set_sensor_mode(port_gyro, "GYRO-G&A");
        sleep(1);
        set_sensor_mode(port_gyro, "GYRO-ANG");
        printf("GYRO sensor calibrated!\n", port_gyro);
        fflush(stdout);
        return 1;
    }
    else
    {
        printf("GYRO sensor is NOT found...\n");
        fflush(stdout);
        return 0;
    }
}

//Get the current gyro angle, with 0 being the angle of the starting position during init
int get_gyro(void)
{
    int val;

    get_sensor_value(0, port_gyro, &val);
    //printf("Gyro value = %d", val);
    //fflush(stdout);

    return val;
}