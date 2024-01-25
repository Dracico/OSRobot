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
int obstacle_found = 0;
int obstacle_distance = 100;
int correctDirection = 1;

// Create a thread and check the sonar to find the flag
void *threadSonarFindFlag(void *vargp)
{
    int gyro = get_gyro();
    int sonar = get_sonar();
    while (true)
    {
        gyro = get_gyro();
        sonar = get_sonar();

        // Get sonar distance and print it to the log
        if (sonar < flag_distance)
        {
            flag_distance = sonar;
            flag_angle = gyro;
            printf("Sonar=%d Angle=%d\n", flag_distance, flag_angle);
            fflush(stdout);
        }
    }
}

// Create a thread and check for obstacles with the sonar
void *threadSonarObstacles(void *vargp)
{
    int sonar = get_sonar();
    while (true)
    {
        sonar = get_sonar();
        if (sonar < obstacle_distance)
        {
            obstacle_found = 1;
            fflush(stdout);
        }
    }
}

// Detect the color in front of the robot
void *threadCountLines(void *vargp)
{
    // White is the base color of the board
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
            // Loop until we have not crossed the different colored line
            while (color != white)
            {
                color = get_color();
                sleep(1);
            }
        }
    }
}

// Main rotate function
void rotateTo(int degrees)
{
    int gyro = get_gyro();

    // While the gyro value is smaller than degrees
    if (gyro < degrees)
    {
        // Turn right
        move_motor(leftWheel, -1, false);
        move_motor(rightWheel, 1, false);
        // Stops a bit earlier because the motors take some time to stop
        while (gyro < (degrees - 8))
        {
            gyro = get_gyro();
        }
    }
    else
    {
        // Turn left
        move_motor(leftWheel, 1, false);
        move_motor(rightWheel, -1, false);
        // Stops a bit earlier because the motors take time to stop
        while (gyro > (degrees + 8))
        {
            gyro = get_gyro();
        }
    }

    // Stop all the motors
    motor_stop();

    gyro = get_gyro();
    // Once the motors are stopped, correct the orientation of the robot to be more precise
    while (gyro < degrees || gyro > degrees)
    {
        // Same as above, if the angle isn't correct, do small rotations to correct the trajectory
        if (gyro < degrees)
        {
            move_motor_angle(leftWheel, -3, false);
            move_motor_angle(rightWheel, 3, true);
        }
        else
        {
            move_motor_angle(leftWheel, 3, false);
            move_motor_angle(rightWheel, -3, true);
        }
        gyro = get_gyro();
    }
}

// Main move forward funct
void moveForward(int direction, int degrees)
{
    // Move both wheels to go forward, then get gyro info
    move_motor(leftWheel, direction * 3, false);
    move_motor(rightWheel, direction * 3, false);
    // Sleep and stop motors in order to not start correcting when they're moving forward
    sleep(1);
    motor_stop();
    int gyro = get_gyro();
    // Readjust the robot to face the correct initial direction
    int stillRunning = 1;
    // While running, if the direction deviates too much, stop and correct
    while (gyro > (degrees + 5) || gyro < (degrees - 5))
    {
        // stops
        if (stillRunning)
        {
            sleep(1);
            stillRunning = 0;
        }
        printf("%d %d\n", gyro, degrees);
        fflush(stdout);
        // Do the correction and wait for motor stop
        if (gyro < degrees)
        {
            move_motor_angle(leftWheel, -3, false);
            move_motor_angle(rightWheel, 3, false);
            wait_motor_stop();
        }
        else
        {
            move_motor_angle(rightWheel, -3, false);
            move_motor_angle(leftWheel, 3, false);
            wait_motor_stop();
        }
        gyro = get_gyro();
    }
}

// Avoid obstacle function
void avoid_obstacles(int direction, int degrees, int initial_value)
{
    // Avoid obstacles
    printf("%d\n", get_sonar());
    fflush(stdout);
    // If an obstacle is found with the sonar

    // Rotate to 90 degrees, move forward, return to original angle and start moving again
    rotateTo(degrees);
    move_motor_angle(leftWheel, 300 * direction, false);
    move_motor_angle(rightWheel, 300 * direction, false);
    wait_motor_stop();
    rotateTo(initial_value);
    correctDirection = correctDirection * -1;

    obstacle_found = 0;
}

// Scan for the flag and correct the trajectory
void scanFlagAndCorrect()
{
    // Make the motors slower
    change_motors_speed(1, 15);
    sleep(1);

    // Turn left and wait for the motor stop
    move_motor_angle(rightWheel, -100, false);
    move_motor_angle(leftWheel, 100, false);
    wait_motor_stop();

    // Start the thread for scanning with the sonar
    pthread_t threadSonarFindFlag_id;
    pthread_create(&threadSonarFindFlag_id, NULL, threadSonarFindFlag, NULL);

    // Turn right and proceed to scanning
    move_motor_angle(leftWheel, -200, false);
    move_motor_angle(rightWheel, 200, false);
    wait_motor_stop();

    pthread_cancel(threadSonarFindFlag_id);
    // Set speed back to original
    change_motors_speed(2, 3);
    // Rotate to face the flag angle
    rotateTo(flag_angle);
}

// Main function
int main(void)
{
    // Set up the different sensors and motors
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

    // Start the thread to detect color changes
    pthread_t threadCountLines_id;
    pthread_t threadSonarObstacles_id;

    // Rotate to 90° to evade the central obstacle
    rotateTo(90);

    // Stopping the arm after a while while save us 4 seconds on boot time
    stop_motor(arm);

    // Go forward until it has crossed 3 lines
    obstacle_distance = 150;
    // change_motors_speed(1, 2);
    pthread_create(&threadSonarObstacles_id, NULL, threadSonarObstacles, NULL);
    while (obstacle_found == 0)
    {
        moveForward(-1, 90);
    }
    pthread_cancel(threadSonarObstacles_id);

    // Rotate on the left in order to aim for the enemy flag
    rotateTo(0);

    // Setup the distance of obstacle to avoid false positive
    obstacle_distance = 100;
    obstacle_found = 0;
    linesCrossed = 0;
    correctDirection = 1;
    pthread_create(&threadSonarObstacles_id, NULL, threadSonarObstacles, NULL);
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);
    while (linesCrossed < 3)
    {
        if (obstacle_found)
        {
            if (correctDirection > 0)
            {
                avoid_obstacles(-1, -90, 0);
            }
            else
            {
                avoid_obstacles(-1, 90, 0);
            }
        }
        moveForward(-1, 0);
    }
    pthread_cancel(threadSonarObstacles_id);
    pthread_cancel(threadCountLines_id);

    move_motor_angle(leftWheel, -300, false);
    move_motor_angle(rightWheel, -300, false);
    wait_motor_stop();
    rotateTo(-90);

    linesCrossed = 0;
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);
    while (linesCrossed < 1)
    {
        moveForward(-1, -90);
    }
    pthread_cancel(threadCountLines_id);
    sleep(1);
    move_motor_angle(leftWheel, -180, false);
    move_motor_angle(rightWheel, -180, false);
    wait_motor_stop();

    // Scanning and reaching for the flag and reaching it (after having crossed the 4th line as pre-condition)
    rotateTo(0);
    scanFlagAndCorrect();
    while (flag_distance > 150)
    {
        // change_motors_speed(10);

        move_motor_angle(leftWheel, -180, false);
        move_motor_angle(rightWheel, -180, false);
        sleep(1);

        scanFlagAndCorrect();
    }

    // Rotate 180º
    rotateTo(180 + flag_angle);

    // Go forward a bit
    move_motor_angle(leftWheel, 300, false);
    move_motor_angle(rightWheel, 300, false);
    wait_motor_stop();

    // Lower the arm
    move_motor_angle(arm, -60, false);
    sleep(1);

    // Go forward a bit
    move_motor_angle(leftWheel, -200, false);
    move_motor_angle(rightWheel, -200, false);

    sleep(2);

    // Lower the arm
    move_motor_angle(arm, -60, false);

    // Go forward a bit
    move_motor_angle(leftWheel, -200, false);
    move_motor_angle(rightWheel, -200, false);
    wait_motor_stop();

    // Go back to the base
    rotateTo(260);
    obstacle_found = 0;
    obstacle_distance = 180;
    pthread_create(&threadSonarObstacles_id, NULL, threadSonarObstacles, NULL);
    while (obstacle_found == 0)
    {
        moveForward(-1, 260);
    }
    pthread_cancel(threadSonarObstacles_id);

    rotateTo(175);

    obstacle_found = 0;
    obstacle_distance = 100;
    linesCrossed = 0;
    correctDirection = -1;
    pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);
    pthread_create(&threadSonarObstacles_id, NULL, threadSonarObstacles, NULL);
    while (linesCrossed < 3)
    {
        if (obstacle_found)
        {
            if (correctDirection > 0)
            {
                avoid_obstacles(-1, 270, 175);
            }
            else
            {
                avoid_obstacles(-1, 90, 175);
            }
        }
        moveForward(-1, 175);
        // avoid_obstacles(-1, 180);
    }
    pthread_cancel(threadSonarObstacles_id);
    pthread_cancel(threadCountLines_id);
    rotateTo(360);

    change_motors_speed(1, 15);
    move_motor_angle(leftWheel, 600, false);
    move_motor_angle(rightWheel, 600, false);
    wait_motor_stop();
    sleep(1);
    move_motor_angle(leftWheel, -80, false);
    move_motor_angle(rightWheel, -80, false);
    wait_motor_stop();

    move_motor_angle(arm, 40, true);
    sleep(1);
    move_motor_angle(leftWheel, 70, false);
    move_motor_angle(rightWheel, 70, false);
    sleep(1);

    rotateTo(355);

    move_motor_angle(arm, 300, false);
    sleep(3);
    stop_motor(arm);
    move_motor_angle(leftWheel, -100, false);
    move_motor_angle(rightWheel, -100, false);

    return 0;
}