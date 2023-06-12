/**
 * @file RemoteDisplay.h
 * @author Marcelo Fraga
 * @brief Header file for RemoteDisplay.c
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef REMOTEDISPLAY_H
#define REMOTEDISPLAY_H
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/Picopixel.h>



/*
* Others
*/

#define REMOTE_DISPLAY_ANALOG_MAX 1023
#define REMOTE_DISPLAY_ANALOG_MIN 0

/*
* OLED display related
*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 


// Draw related defines
// Joystick
#define JOYSTICK_BLANK_SPACE           10
#define JOYSTICK_DRAW_RADIUS           15
#define JOYSTICK_LEFT_DRAW_POSITION_X  (uint16_t)((SCREEN_WIDTH * 0.25f) + JOYSTICK_BLANK_SPACE)
#define JOYSTICK_LEFT_DRAW_POSITION_Y  30
#define JOYSTICK_RIGHT_DRAW_POSITION_X (uint16_t)((SCREEN_WIDTH - (SCREEN_WIDTH * 0.25f)) - JOYSTICK_BLANK_SPACE)
#define JOYSTICK_RIGHT_DRAW_POSITION_Y JOYSTICK_LEFT_DRAW_POSITION_Y

#define JOYSTICK_BALL_RADIUS 3

// Battery indication
#define BATTERY_INDICATION_CHARGE_BLANK 2
#define BATTERY_INDICATION_WIDTH      15
#define BATTERY_INDICATION_HEIGHT     8 
#define BATTERY_INDICATION_POSITION_X SCREEN_WIDTH - (BATTERY_INDICATION_WIDTH + BATTERY_INDICATION_CHARGE_BLANK)
#define BATTERY_INDICATION_POSITION_Y 0

#define BATTERY_INDICATION_CHARGE_POSITION_X BATTERY_INDICATION_POSITION_X + BATTERY_INDICATION_CHARGE_BLANK
#define BATTERY_INDICATION_CHARGE_POSITION_Y BATTERY_INDICATION_POSITION_Y + BATTERY_INDICATION_CHARGE_BLANK
#define BATTERY_INDICATION_CHARGE_HEIGHT BATTERY_INDICATION_HEIGHT - BATTERY_INDICATION_CHARGE_BLANK*2  // Battery charge width is calculated using percentage
#define BATTERY_INDICATION_CHARGE_MIN 0
#define BATTERY_INDICATION_CHARGE_MAX BATTERY_INDICATION_WIDTH - BATTERY_INDICATION_CHARGE_BLANK-1

// Communication
#define N_TICKS 3
#define COMMUNICATION_TICK_SIZE  3 // The size of the smallest communication tick.   .1| As an example, tick is the dot
#define COMMUNICATION_TICK_WIDTH 2
#define COMMUNICATION_TICK_SPACE 4
#define NO_COMMUNICATION_BLINK_TIME 1000 // in ms



extern Adafruit_SSD1306 display;

typedef struct OLEDJoystickPosition
{
    int8_t x;
    int8_t y;
}OLEDJoystickPosition;

class RemoteDisplay
{
    private:
        OLEDJoystickPosition ComputeOLEDJoystickPosition(uint16_t x, uint16_t y);


    public:
        RemoteDisplay();
        void printJoystickOLED(uint16_t jx_l, uint16_t jy_l, uint16_t jx_r, uint16_t jy_r);
        void printBatteryOLED(float battery_percentage);
        void printConnectionStatusOLED(int tx_time, bool no_connection);

};


#endif