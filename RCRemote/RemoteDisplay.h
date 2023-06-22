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


typedef struct View_t_Input
{
    bool b_ButtonClicked;
    bool b_ButtonMoveDown;
    bool b_ButtonMoveUp;
}View_t_Input;

#define MAX_COMPONENT_CHAR 3u
#define MAX_INPUT_COMPONENTS_PER_VIEW 3u
#define MAX_COMPONENTS_PER_VIEW 7u

class Component // 13bytes down to 3bytes after removing the strcpy //16 bytes wth flashstring??
{
    protected:
        Adafruit_SSD1306* display_obj;  
        // char              c_name[MAX_COMPONENT_CHAR];´
        uint8_t           u8_x, u8_y;

    public:
        Component(Adafruit_SSD1306* display_obj)
        : display_obj(display_obj)
        {
            display_obj->setTextSize(1);             // Normal 1:1 pixel scale
            display_obj->setTextColor(WHITE);
            // strncpy(c_name, name, MAX_COMPONENT_CHAR);
            // c_name[MAX_COMPONENT_CHAR - 1] = '\0';
        }
        Component() = default;

        virtual void draw()
        {
            display_obj->print(F("Nothing to draw :)"));
        }

        virtual void update(uint16_t value)
        {
            display_obj->print(F("Something went wront updating :)"));
        }

        void setPosition(uint8_t x, uint8_t y)
        {
            u8_x = x;
            u8_y = y;
        }
};

class Button: public Component // 21 bytes
{
    private:
        bool b_isSelected;
        bool b_isClicked;

    public:
        Button(Adafruit_SSD1306* display_obj);
        Button() = default;
        void draw();
        void update(bool input, bool currentlySelected); // Updates isSelected and isClicked
};

/* Monitors and shows an analog value on the screen.*/
class AnalogMonitor: public Component // 23 bytes down to 19bytes
{
    private:
        uint16_t u16_value;

    public:
        AnalogMonitor(Adafruit_SSD1306* display_obj);
        AnalogMonitor() = default;
        void draw() override;
        void update(uint16_t value) override;

};

class View // 47 bytes, 24bytes after removing component
{  


    private:
        bool b_currentlyViewing;
        
        uint8_t u8_Component_Idx, u8_Button_Idx;
        uint8_t u8_Currently_Selected_Button;
        
        Button*    e_InputComponents[MAX_INPUT_COMPONENTS_PER_VIEW];
        Component* e_Components[MAX_COMPONENTS_PER_VIEW];

    public:
        View();
        void draw();
        void update(View_t_Input b);
        void addComponent(Component* c);
        void addButton(Button* b);
};

// class RemoteDisplay
// {
//     private:


//     public:
//         RemoteDisplay();
//         void printJoystickOLED(uint16_t jx_l, uint16_t jy_l, uint16_t jx_r, uint16_t jy_r);
//         void printBatteryOLED(float battery_percentage);
//         void printConnectionStatusOLED(int tx_time, bool no_connection);

// };


#endif