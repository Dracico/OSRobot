#include "../include/motors.h"

//Possibly not working ?
#define Sleep(msec) usleep((msec) * 1000)

//Create the 3 ID variables of our motors
uint8_t arm = -1;        // Port A
uint8_t rightWheel = -1; // Port B
uint8_t leftWheel = -1;  // Port C

//Setup the motors. Wait for general initialization before continuing with the function
bool setup_motors(void)
{
    while (ev3_tacho_init() < 1)
    {
        Sleep(1000);
    }

    int max_speed;

    // Find the sequence number of each wheel motors (L motor)
    uint8_t sn = -1;
    while (ev3_search_tacho(LEGO_EV3_L_MOTOR, &sn, sn + 1))
    {
        char s[256];
        get_tacho_address(sn, s, sizeof(s));
        if (strcmp(s, "ev3-ports:outC") == 0)
        {
            leftWheel = sn;
        }
        if (strcmp(s, "ev3-ports:outB") == 0)
        {
            rightWheel = sn;
        }
    }

    // Find the arm sequence number (we only have one M motor)
    ev3_search_tacho(LEGO_EV3_M_MOTOR, &sn, 0);
    arm = sn;

    // Verify that all sequence number have been assigned
    if (arm == -1)
    {
        printf("Arm not connected!");
        return false;
    }
    if (leftWheel == -1)
    {
        printf("Left wheel not connected!");
        return false;
    }
    if (rightWheel == -1)
    {
        printf("Right wheel not connected!");
        return false;
    }

    //Setup the attributes of each motors such as acceleration or max speed.
    int arr[] = {arm, leftWheel, rightWheel};
    for (int i = 0; i <= 2; i++)
    {
        sn = arr[i];
        if (sn != arm)
        {
            printf("Wheel speed setup\n");
            get_tacho_max_speed(sn, &max_speed);
            set_tacho_stop_action_inx(sn, TACHO_BRAKE);
            set_tacho_speed_sp(sn, max_speed * 2 / 3);
            set_tacho_time_sp(sn, 5000);
            set_tacho_ramp_up_sp(sn, 2000);
            set_tacho_ramp_down_sp(sn, 2000);
        }
        else
        {
            printf("Arm speed setup\n");
            get_tacho_max_speed(sn, &max_speed);
            set_tacho_stop_action_inx(sn, TACHO_BRAKE);
            set_tacho_command_inx(sn, TACHO_STOP);
            set_tacho_speed_sp(sn, 100);
        }
    }

    return true;
}

// Change the speed of the wheel motors. Quotient is the divider of the speed. 
// In our case, we always use the same speed for both motors as instead use wheel direction to turn.
bool change_motors_speed(int multiplier, int quotient)
{
    int max_speed;

    printf("Right Wheel speed setup\n");
    get_tacho_max_speed(rightWheel, &max_speed);
    set_tacho_speed_sp(rightWheel, max_speed * multiplier / quotient);

    printf("Left Wheel speed setup\n");
    get_tacho_max_speed(leftWheel, &max_speed);
    set_tacho_speed_sp(leftWheel, max_speed * multiplier / quotient);

    return true;
}

// Change the speed of the motors. Quotient is the divider of the speed. (Arm speed function)
bool change_arm_speed(int value)
{
    set_tacho_speed_sp(arm, value);
    return true;
}

//Function to move motors by time. Direction should be positive for going backward, and negative to go forward.
//Blocking is used if we want to prevent the program to do other actions during the motors moving. Allows us to handle
// cases when we wants the motor to stop before using another motor (For example, move forward, stop then use the arm)
void move_motor(uint8_t sn, int direction, bool blocking)
{
    FLAGS_T state;

    set_tacho_speed_sp(sn, 100 * direction);
    set_tacho_command_inx(sn, TACHO_RUN_TIMED);
    if (blocking)
    {
        do
        {
            get_tacho_state_flags(sn, &state);
        } while (state != 2 && state != 0);
    }
}

//Similar as function above, except the moving is done by angle of the motor (Not very precise) and not by time.
void move_motor_angle(uint8_t sn, int degrees, bool blocking)
{
    FLAGS_T state;

    set_tacho_position_sp(sn, degrees);
    set_tacho_command_inx(sn, TACHO_RUN_TO_REL_POS);
    if (blocking)
    {
        do
        {
            get_tacho_state_flags(sn, &state);
        } while (state != 2 && state != 0);
    }
}

//Used to stop the motors. Useful when we need to stop the wheel faster or stop the arm from staying in a blocked position
void stop_motor(uint8_t sn)
{
    set_tacho_command_inx(sn, TACHO_STOP);
}

//Hold the program until both wheel potors are stopped, allows us a greater precision for the arm by making sure the
//Robot is not moving anymore
void wait_motor_stop()
{
    FLAGS_T stateLeft, stateRight;
    do
    {
        get_tacho_state_flags(leftWheel, &stateLeft);
        get_tacho_state_flags(rightWheel, &stateRight);
    } while ((leftWheel != 2 && leftWheel != 0) || (stateRight != 2 && stateRight != 0));
}

//Other version of stop motor, we are getting better result by combining both Techo hold and tacho stop.
void motor_stop()
{
    set_tacho_command_inx(rightWheel, TACHO_HOLD);
    set_tacho_command_inx(leftWheel, TACHO_HOLD);
    set_tacho_command_inx(rightWheel, TACHO_STOP);
    set_tacho_command_inx(leftWheel, TACHO_STOP);
}