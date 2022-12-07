#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "motordriver.h"
#include "XNucleo53L0A1.h"
#include <stdio.h>

/// Pin Assignments ///
 // uLCD
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;
// left motor
Motor  left(p21, p22, p23, 1); // pwm, fwd, rev, has brake feature
// right motor
Motor right(p26, p25, p24, 1);
// speaker with amp
AnalogOut speaker(p18);
//I2C sensor pins lidar
DigitalOut shdn(p29);
#define VL53L0_I2C_SDA   p28
#define VL53L0_I2C_SCL   p27
static XNucleo53L0A1 *board=NULL;
// rpg
AnalogIn rpg(p15);

/// Variables ///
int status;
uint32_t distance;
bool alarm = false;
int current_hour = 0;
int current_min = 0;
int alarm_hour = 0;
int alarm_min = 0;

// might need mutex

void incTime()
{
    while(1)
    {
        current_min++;
        if (current_min == 60)
        {
            current_min = 0;
            current_hour++;
        }
        if (current_hour == 24)
        {
            current_hour = 0;
        }
        if (current_hour == alarm_hour && current_min == alarm_min)
        {
            alarm = true;
        }
        Thread::wait(60 * 1000);
    }
}

// play speaker if it is alarm time. check time as well
void playSpeaker()
{
    while(1)
    {
        if (alarm)
        {
            speaker = 0.5;
        }
        else
        {
            speaker = 0;
        }
        Thread::wait(1000);
    }
}

void turn()
{
    left.speed(0);
    right.speed(0);
    Thread::wait(1);
    left.speed(-0.5);
    right.speed(0.5);
    Thread::wait(1);
    left.speed(0);
    right.speed(0);
    Thread::wait(1);
    left.speed(0.5);
    right.speed(0.5);
}

// check sonar/lidar if we need to turn and move forward
void movement()
{
    while(1)
    {
        while (alarm)
        {
            status = board->sensor_centre->get_distance(&::distance);
            if (::distance <= 25)
            {
                turn();
            }
            else
            {
                left.speed(0.5);
                right.speed(0.5);
            }
        }
        Thread::wait(1000);
    }
}

int main() {
    /// initialize everything ///
    // init lidar //
    DevI2C *device_i2c = new DevI2C(VL53L0_I2C_SDA, VL53L0_I2C_SCL);
    /* creates the 53L0A1 expansion board singleton obj */
    board = XNucleo53L0A1::instance(device_i2c, A2, D8, D2);
    shdn = 0; //must reset sensor for an mbed reset to work
    wait(0.1);
    shdn = 1;
    wait(0.1);
    /* init the 53L0A1 board with default values */
    status = board->init_board();
    // init uLCD //
    
    
    /// Start Threads ///
    Thread t1(incTime);
    Thread t2(playSpeaker);
    Thread t3(movement);
    
    // update uLCD in main thread
    // rpg will also change time settings
    while(1)
    {
        
        Thread::wait(1000);
    }
}
