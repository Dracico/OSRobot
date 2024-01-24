#include "../include/motors.h"

#define Sleep(msec) usleep((msec) * 1000)

uint8_t arm = -1;        // Port A
uint8_t rightWheel = -1; // Port B
uint8_t leftWheel = -1;  // Port C

bool setup_motors(void)
{
    while (ev3_tacho_init() < 1)
    {
        Sleep(1000);
    }

    int max_speed;

    // Find which sequence number each motor corresponds to based on the port it is connected
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

// Change the speed of the motors. Quotient is the divider of the speed
bool change_motors_speed(int multiplier, int quotient)
{
    int max_speed;

    printf("Right Wheel speed setup\n");
    get_tacho_max_speed(rightWheel, &max_speed);
    set_tacho_speed_sp(rightWheel, max_speed * multiplier / quotient);

    printf("Right Wheel speed setup\n");
    get_tacho_max_speed(leftWheel, &max_speed);
    set_tacho_speed_sp(leftWheel, max_speed * multiplier / quotient);

    return true;
}

// Change the speed of the motors. Quotient is the divider of the speed
bool change_arm_speed(int value)
{
    set_tacho_speed_sp(arm, value);
    return true;
}
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

void stop_motor(uint8_t sn)
{
    set_tacho_command_inx(sn, TACHO_STOP);
}

void wait_motor_stop()
{
    FLAGS_T stateLeft, stateRight;
    do
    {
        get_tacho_state_flags(leftWheel, &stateLeft);
        get_tacho_state_flags(rightWheel, &stateRight);
    } while ((leftWheel != 2 && leftWheel != 0) || (stateRight != 2 && stateRight != 0));
}

void motor_stop()
{
    set_tacho_command_inx(rightWheel, TACHO_HOLD);
    set_tacho_command_inx(leftWheel, TACHO_HOLD);
    set_tacho_command_inx(rightWheel, TACHO_STOP);
    set_tacho_command_inx(leftWheel, TACHO_STOP);
}