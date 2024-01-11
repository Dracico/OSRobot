#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
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
    move_motor_angle(leftWheel, -420, false);
    move_motor_angle(rightWheel, 420, false);
    while (gyro < (degrees - 1) || gyro > (degrees + 1))
    {
        // Move both wheels to go forward, then get gyro info
        if (gyro < degrees)
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
        printf("%d %d\n", gyro, degrees);
        fflush(stdout);
        if (gyro < degrees)
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

void scanFlagAndCorrect()
{

    change_motors_speed(1, 15);

    move_motor_angle(leftWheel, 100, false);
    move_motor_angle(rightWheel, -100, false);
    wait_motor_stop();

    pthread_t threadSonarFindFlag_id;
    pthread_create(&threadSonarFindFlag_id, NULL, threadSonarFindFlag, NULL);
    sleep(1);

    move_motor_angle(leftWheel, -200, false);
    move_motor_angle(rightWheel, 200, false);
    wait_motor_stop();

    pthread_cancel(threadSonarFindFlag_id);

    move_motor_angle(leftWheel, 100, false);
    move_motor_angle(rightWheel, -100, false);
    wait_motor_stop();

    // Readjust the robot to face the flag
    int gyro = get_gyro();
    while (gyro > (flag_angle + 1) || gyro < (flag_angle - 1))
    {
        printf("%d %d\n", gyro, flag_angle);
        fflush(stdout);
        if (gyro > flag_angle)
        {
            move_motor_angle(leftWheel, 5, false);
            move_motor_angle(rightWheel, -5, false);
        }
        else
        {
            move_motor_angle(leftWheel, -5, false);
            move_motor_angle(rightWheel, 5, false);
        }

        sleep(1);
        gyro = get_gyro();
    }
    sleep(1);
    change_motors_speed(2, 3);
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

    // Scanning and reaching for the flag and reaching it (after having crossed the 4th line as pre-condition)
    scanFlagAndCorrect();
    while (flag_distance > 150)
    {
        // change_motors_speed(10);

        move_motor_angle(leftWheel, -150, false);
        move_motor_angle(rightWheel, -150, false);
        sleep(1);

        scanFlagAndCorrect();
    }

    // Rotate 180º
    rotateTo(180 + flag_angle + 5);

    // Go back a bit
    move_motor_angle(leftWheel, 400, false);
    move_motor_angle(rightWheel, 400, false);
    wait_motor_stop();

    // Lower the arm
    move_motor_angle(arm, -120, true);

    // Go back a bit
    move_motor_angle(leftWheel, -100, false);
    move_motor_angle(rightWheel, -100, false);

    sleep(1);

    // Lower the arm
    move_motor_angle(arm, -40, false);

    // Go back a bit
    move_motor_angle(leftWheel, -500, false);
    move_motor_angle(rightWheel, -500, false);

    // Start thread again
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);

    // Go back to the base
    while (linesCrossed < 4)
    {
        moveForward(-1, 180);
    }
    pthread_cancel(threadCountLines_id);

    rotateTo(0);

    moveForward(1, 0);
    moveForward(1, 0);

    // Raise the arm (just in case its down)
    move_motor_angle(arm, 400, false);
    move_motor_angle(leftWheel, -30, false);
    move_motor_angle(rightWheel, -30, false);
    sleep(3);
    stop_motor(arm);

    return 0;
}