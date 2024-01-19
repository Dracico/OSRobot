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

// Detect the flag in front of the robot
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

    if (gyro < degrees)
    {
        move_motor(leftWheel, -1, false);
        move_motor(rightWheel, 1, false);
        while (gyro < (degrees - 8))
        {
            gyro = get_gyro();
        }
    }
    else
    {
        move_motor(leftWheel, 1, false);
        move_motor(rightWheel, -1, false);
        while (gyro > (degrees + 8))
        {
            gyro = get_gyro();
        }
    }

    motor_stop();

    gyro = get_gyro();
    while (gyro < degrees || gyro > degrees)
    {
        // Move both wheels to go forward, then get gyro info
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

void moveForward(int direction, int degrees)
{
    // Move both wheels to go forward, then get gyro info
    move_motor(leftWheel, direction * 3, false);
    move_motor(rightWheel, direction * 3, false);
    sleep(1);
    motor_stop();
    int gyro = get_gyro();
    // Readjust the robot to face the correct initial direction
    while (gyro > (degrees + 2) || gyro < (degrees - 2))
    {
        printf("%d %d\n", gyro, degrees);
        fflush(stdout);
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

void avoid_obstacles(int direction, int degrees)
{
    // Avoid obstacles
    printf("%d\n", get_sonar());
    fflush(stdout);
    if (obstacle_found)
    {
        rotateTo(degrees - 90);
        move_motor(leftWheel, direction * 4, false);
        move_motor(rightWheel, direction * 4, false);
        sleep(1);
        motor_stop();
        rotateTo(degrees);
    }
    obstacle_found = 0;
}

void scanFlagAndCorrect()
{
    change_motors_speed(1, 15);
    sleep(1);

    move_motor_angle(rightWheel, -100, false);
    move_motor_angle(leftWheel, 100, false);
    wait_motor_stop();

    pthread_t threadSonarFindFlag_id;
    pthread_create(&threadSonarFindFlag_id, NULL, threadSonarFindFlag, NULL);

    move_motor_angle(leftWheel, -200, false);
    move_motor_angle(rightWheel, 200, false);
    wait_motor_stop();

    pthread_cancel(threadSonarFindFlag_id);
    change_motors_speed(2, 3);

    rotateTo(flag_angle);
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
    /*  move_motor_angle(arm, 400, false);
      sleep(3);
      stop_motor(arm);

      // Start the thread to detect color changes
      pthread_t threadCountLines_id;
      pthread_create(&threadCountLines_id, NULL, threadCountLines, NULL);
      pthread_t threadSonarObstacles_id;
      pthread_create(&threadSonarObstacles_id, NULL, threadSonarObstacles, NULL);

      rotateTo(20);
      while (linesCrossed < 3)
      {
          moveForward(-1, 20);
      }

      rotateTo(-25);

      obstacle_distance = 300;
      while (linesCrossed < 5)
      {
          avoid_obstacles(-1, -25);
          moveForward(-1, -25);
      }
      rotateTo(0);

      // Reset line count
      linesCrossed = 0;

      // Scanning and reaching for the flag and reaching it (after having crossed the 4th line as pre-condition)
      scanFlagAndCorrect();
      while (flag_distance > 150)
      {
          // change_motors_speed(10);

          move_motor_angle(leftWheel, -200, false);
          move_motor_angle(rightWheel, -200, false);
          sleep(1);

          scanFlagAndCorrect();
      }

      // Rotate 180º
      rotateTo(180 + flag_angle + 4);

      // Go back a bit
      move_motor_angle(leftWheel, 300, false);
      move_motor_angle(rightWheel, 300, false);
      wait_motor_stop();

      // Lower the arm
      move_motor_angle(arm, -60, false);
      sleep(1);

      // Go back a bit
      move_motor_angle(leftWheel, -100, false);
      move_motor_angle(rightWheel, -100, false);

      sleep(1);

      // Lower the arm
      move_motor_angle(arm, -40, false);

      // Go forward a bit
      move_motor_angle(leftWheel, -200, false);
      move_motor_angle(rightWheel, -200, false);
      wait_motor_stop();

      // Go back to the base
      rotateTo(270);
      obstacle_found = 0;
      obstacle_distance = 300;
      while (obstacle_found == 0)
      {
          moveForward(-1, 270);
      }

      rotateTo(180);
      obstacle_found = 0;
      linesCrossed = 0;
      while (linesCrossed < 3)
      {
          moveForward(-1, 180);
          avoid_obstacles(-1, 180);
      }
      rotateTo(0);
  */
    change_motors_speed(1, 10);
    move_motor_angle(leftWheel, 500, false);
    move_motor_angle(rightWheel, 500, false);
    wait_motor_stop();
    sleep(1);
    move_motor_angle(leftWheel, -100, false);
    move_motor_angle(rightWheel, 100, false);
    wait_motor_stop();

    move_motor_angle(arm, 100, true);
    move_motor_angle(leftWheel, 50, false);
    move_motor_angle(rightWheel, 50, false);
    move_motor_angle(arm, 300, false);
    sleep(3);
    stop_motor(arm);
    move_motor_angle(leftWheel, -100, false);
    move_motor_angle(rightWheel, -100, false);

    return 0;
}