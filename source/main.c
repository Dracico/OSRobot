#include <stdio.h>
#include <pthread.h>
#include "../ev3dev-c/source/ev3/ev3.h"
#include "../include/touch.h"
#include "../include/gyro.h"
#include "../include/color.h"
#include "../include/sonar.h"
#include "../include/motors.h"

int linesCrossed = 0;
int flag_distance = 1000;
int flag_angle = 0;

// Detect the color in front of the robot
void *sonarFindFlag(void *vargp)
{
    int gyro = get_gyro();
    int sonar = get_sonar();
    while (true)
    {
        gyro = get_gyro();
        sonar = get_sonar();

        if (sonar < flag_distance)
        {
            flag_distance = sonar;
            flag_angle = gyro;
            printf("Sonar=%d Angle=%d\n", flag_distance, flag_angle);
            fflush(stdout);
        }
    }
}

// Detect the color in front of the robot
void *mainColor(void *vargp)
{
    int white = 6;
    while (true)
    {
        int color = get_color();
        // Increment the linesCrossed variable once the colour is
        // Different from white
        if (color != white)
        {
            printf("%d\n", color);
            fflush(stdout);
            linesCrossed++;

            while (color != white)
            {
                color = get_color();
                sleep(1);
            }
        }
    }
}

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

    if (!init_sensors())
    {
        printf("Can't setup sensors!");
        return 1;
    }

    init_gyro();
    init_color();
    init_sonar();

    // Raise the arm (just in case its down)
    move_motor(arm, 1, true);

    // Start the thread to detect color changes
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, mainColor, NULL);

    // While the robot hasn't crossed the correct number of line
    /*while (linesCrossed < 4)
    {
        //Move both wheels to go forward, then get gyro info
        move_motor_angle(leftWheel, -360, false);
        move_motor_angle(rightWheel, -360, false);
        wait_motor_stop();
        sleep(1);
        int gyro = get_gyro();
        //Readjust the robot to face the correct initial direction
        while (gyro > 3 || gyro < -3)
        {
            printf("%d\n", gyro);
            fflush(stdout);
            if (gyro < 0)
            {
                move_motor_angle(leftWheel, -10, false);
            }
            else
            {
                move_motor_angle(rightWheel, -10, false);
            }
            // usleep(200);
            sleep(1);

            gyro = get_gyro();
        }
    }*/

    // Move 15º to the right
    int gyro = get_gyro();
    while (gyro > -15)
    {
        move_motor_angle(leftWheel, 50, false);
        move_motor_angle(rightWheel, -50, false);
        sleep(1);
        gyro = get_gyro();
    }

    // Sweep 30º to the left to find the flag
    pthread_t thread_sonarFindFlag;
    pthread_create(&thread_sonarFindFlag, NULL, sonarFindFlag, NULL);
    while (gyro < 15)
    {
        move_motor_angle(leftWheel, -10, false);
        move_motor_angle(rightWheel, 10, false);
        sleep(1);
        gyro = get_gyro();
    }
    pthread_cancel(thread_sonarFindFlag);

    /*
        // Move to the position of the flag we've found!
        while (gyro < flag_angle)
        {
            move_motor_angle(leftWheel, -5, false);
            move_motor_angle(rightWheel, 5, false);
            sleep(1);
            gyro = get_gyro();
        }*/

    // Get closer to the flag
    /*int sonar = get_sonar();
    while (sonar > 200)
    {
        // Move both wheels to go forward, then get gyro info
        move_motor_angle(leftWheel, -20, false);
        move_motor_angle(rightWheel, -20, false);
        sleep(1);
        sonar = get_sonar();
        printf("%d\n", sonar);
        fflush(stdout);
    }*/

    // Rotate to the back!
    gyro = get_gyro();
    move_motor_angle(leftWheel, -360, false);
    move_motor_angle(rightWheel, 360, false);
    while (gyro < (180 + flag_angle - 1) || gyro > (180 + flag_angle + 1))
    {
        // Move both wheels to go forward, then get gyro info
        if (gyro < (180 + flag_angle))
        {
            move_motor_angle(leftWheel, -10, false);
            move_motor_angle(rightWheel, 10, false);
        }
        else
        {
            move_motor_angle(leftWheel, 10, false);
            move_motor_angle(rightWheel, -10, false);
        }

        sleep(1);
        gyro = get_gyro();
        printf("%d\n", gyro);
        fflush(stdout);
    }

    // Go back a bit!
    move_motor_angle(leftWheel, 300 + flag_distance, false);
    move_motor_angle(rightWheel, 300 + flag_distance, true);

    // Lower the arm
    move_motor(arm, -1, true);

    // Go forward!
    move_motor_angle(leftWheel, -360, false);
    move_motor_angle(rightWheel, -360, false);
    sleep(1);

    /*
    // Raise the arm (just in case its down)
    move_motor(arm, 1, true);

    // Pick up the flag and wait for the arm to stop moving
    move_motor(arm, -1, true);

    // Move backwards (both wheels at the same time)
    move_motor_angle(leftWheel, -360, false);
    move_motor_angle(rightWheel, -360, false);
*/

    /*
        init_sonar();
        while (true)
        {
            printf("%d\n", get_sonar());
            fflush(stdout);
            sleep(1);
        }
        */

    /*init_color();
    while (true)
    {
        printf("%d\n", get_color());
        fflush(stdout);
        sleep(1);
    }*/

    /*init_gyro();
    while (true)
    {
        printf("%d\n", get_gyro());
        fflush(stdout);
        sleep(1);
    }*/

    return 0;
}