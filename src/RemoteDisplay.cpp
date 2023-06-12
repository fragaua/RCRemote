/**
 * @file RemoteDisplay.c
 * @author Marcelo Fraga
 * @brief Contains all functions for writting to the OLED screen as well as potential logic for menus.
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "RemoteDisplay.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


OLEDJoystickPosition RemoteDisplay::ComputeOLEDJoystickPosition(uint16_t jx, uint16_t jy)
{
  OLEDJoystickPosition joystick_oled_pos;
  joystick_oled_pos.x = (int8_t)map(jx, REMOTE_DISPLAY_ANALOG_MIN, REMOTE_DISPLAY_ANALOG_MAX, -JOYSTICK_DRAW_RADIUS, JOYSTICK_DRAW_RADIUS);
  joystick_oled_pos.y = (int8_t)map(jy, REMOTE_DISPLAY_ANALOG_MIN, REMOTE_DISPLAY_ANALOG_MAX, -JOYSTICK_DRAW_RADIUS, JOYSTICK_DRAW_RADIUS);
  return joystick_oled_pos;
}

RemoteDisplay::RemoteDisplay()
{
  display.setFont(&Picopixel);
}

void RemoteDisplay::printJoystickOLED(uint16_t jx_l, uint16_t jy_l, uint16_t jx_r, uint16_t jy_r)
{
  // Doesn't seem necessary, creating functions to do both sides. Will do them both here.

  display.drawCircle(JOYSTICK_LEFT_DRAW_POSITION_X, JOYSTICK_LEFT_DRAW_POSITION_Y, JOYSTICK_DRAW_RADIUS, WHITE);  
  display.drawCircle(JOYSTICK_RIGHT_DRAW_POSITION_X, JOYSTICK_RIGHT_DRAW_POSITION_Y, JOYSTICK_DRAW_RADIUS, WHITE);  
  
  // Map the original joystick values to pixel values inside the joystick area on screen
  OLEDJoystickPosition joystick_oled_position_l = ComputeOLEDJoystickPosition(jx_l, jy_l);
  OLEDJoystickPosition joystick_oled_position_r = ComputeOLEDJoystickPosition(jx_r, jy_r);
  display.fillCircle(JOYSTICK_LEFT_DRAW_POSITION_X - joystick_oled_position_l.y, JOYSTICK_LEFT_DRAW_POSITION_Y + joystick_oled_position_l.x, 
                     JOYSTICK_BALL_RADIUS, WHITE);
  display.fillCircle(JOYSTICK_RIGHT_DRAW_POSITION_X - joystick_oled_position_r.y, JOYSTICK_RIGHT_DRAW_POSITION_Y + joystick_oled_position_r.x, 
                     JOYSTICK_BALL_RADIUS, WHITE);
  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE); 
  display.setCursor(JOYSTICK_LEFT_DRAW_POSITION_X-12, JOYSTICK_LEFT_DRAW_POSITION_Y + JOYSTICK_DRAW_RADIUS*1.5);
  display.print(jx_l);
  display.print(F(","));
  display.print(jy_l);
  display.setCursor(JOYSTICK_RIGHT_DRAW_POSITION_X-12, JOYSTICK_RIGHT_DRAW_POSITION_Y + JOYSTICK_DRAW_RADIUS*1.5);
  display.print(jx_r);
  display.print(F(","));
  display.print(jy_r);


}

void RemoteDisplay::printBatteryOLED(float battery_percentage)
{
  uint8_t fill_width = map(battery_percentage, 0.0f, 100.0f, BATTERY_INDICATION_CHARGE_MIN, BATTERY_INDICATION_CHARGE_MAX);
  display.fillRect(BATTERY_INDICATION_POSITION_X + BATTERY_INDICATION_WIDTH, 
                  BATTERY_INDICATION_POSITION_Y + (BATTERY_INDICATION_HEIGHT*0.25f), 
                  BATTERY_INDICATION_CHARGE_BLANK, (BATTERY_INDICATION_HEIGHT*0.5f), WHITE); // The little rectangle of a battery :)
  
  display.fillRect(BATTERY_INDICATION_CHARGE_POSITION_X, BATTERY_INDICATION_CHARGE_POSITION_Y, fill_width, BATTERY_INDICATION_CHARGE_HEIGHT, WHITE);
  display.drawRect(BATTERY_INDICATION_POSITION_X, BATTERY_INDICATION_POSITION_Y, BATTERY_INDICATION_WIDTH, BATTERY_INDICATION_HEIGHT, WHITE);
  
  display.setFont(&Picopixel);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);
  display.setCursor(BATTERY_INDICATION_POSITION_X-15, BATTERY_INDICATION_POSITION_Y+5);
  display.print(battery_percentage, 1);
  // display.setFont();
}


void RemoteDisplay::printConnectionStatusOLED(int tx_time, bool no_connection)
{
  static bool b_Blink = false;
  static uint32_t u32_TimePassed = millis();

 
  for(uint8_t i = 0; i < N_TICKS; i++)
  {
    display.fillRect(COMMUNICATION_TICK_SIZE*i, COMMUNICATION_TICK_SIZE*(N_TICKS-i-1), COMMUNICATION_TICK_WIDTH, COMMUNICATION_TICK_SIZE*i+1, WHITE);
  }

  display.setFont(&Picopixel);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);
  display.setCursor(COMMUNICATION_TICK_WIDTH*5, 5);
  if(!no_connection) // Only write the tx time if there is connection 
  {
 
    display.print((float)(tx_time * 0.001f));
    display.print(F("ms"));
  }
  else // In case no communication, just print No communication and blink with 1sec
  {
    if(b_Blink)
    {
      display.print(F("No comm!"));
    }
    
    if(millis() - u32_TimePassed > 1000)
    {
      u32_TimePassed = millis(); // Save current time
      b_Blink = !b_Blink;      
    }
  }

  
}

