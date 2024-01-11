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
void *threadSonarFindFlag(void *vargp)
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
void *threadCountLines(void *vargp)
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
            printf("Line crossed %d\n", linesCrossed);
            fflush(stdout);

            while (color != white)
            {
                color = get_color();
            }
        }
    }
}

void rotateTo(int degrees)
{
    int gyro = get_gyro();
    move_motor_angle(leftWheel, -360, false);
    move_motor_angle(rightWheel, 360, false);
    while (gyro < (degrees - 1) || gyro > (degrees + 1))
    {
        // Move both wheels to go forward, then get gyro info
        if (degrees < 180)
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
    }
}

void moveForward(int direction, int degrees)
{
    // Move both wheels to go forward, then get gyro info
    move_motor_angle(leftWheel, 360 * direction, false);
    move_motor_angle(rightWheel, 360 * direction, true);
    int gyro = get_gyro();
    // Readjust the robot to face the correct initial direction
    while (gyro > degrees || gyro < degrees)
    {
        if (gyro < 0)
        {
            move_motor_angle(leftWheel, -5, false);
        }
        else
        {
            move_motor_angle(rightWheel, -5, false);
        }
        sleep(1);
        gyro = get_gyro();
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
    move_motor_angle(arm, 400, false);
    sleep(3);
    stop_motor(arm);
    move_motor_angle(arm, -120, true);
    move_motor_angle(arm, -40, true);

    // Start the thread to detect color changes
    pthread_t threadCountLines_id;
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);

    // While the robot hasn't crossed the correct number of line
    while (linesCrossed < 4)
    {
        moveForward(-1, 0);
    }

    // Kill thread
    pthread_cancel(threadCountLines_id);
    // Reset line count
    linesCrossed = 0;

    // Rotate 180º
    rotateTo(180);

    // Go back a bit
    move_motor_angle(leftWheel, 500, false);
    move_motor_angle(rightWheel, 500, true);

    // Lower the arm
    move_motor_angle(arm, -140, false);

    // Start thread again
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);

    // Go back to the base
    while (linesCrossed < 5)
    {
        moveForward(-1, 180);
    }

    /*
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

    */
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