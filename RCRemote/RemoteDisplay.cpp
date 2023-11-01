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


/* Helper functions */

/* Typically faster than mod and avoids incrementing the indices until overflow*/
uint8_t u_AddRoundRobinIdx(uint8_t idx, uint8_t max_val)
{
  return (idx == max_val-1) ? 0 : idx + 1;
}

uint8_t u_SubRoundRobinIdx(uint8_t idx, uint8_t max_val)
{
  return (idx == -1) ? (max_val-1): idx - 1;
}


AnalogMonitor::AnalogMonitor(Adafruit_SSD1306* display_obj)
                            :Component(display_obj)
{
  u16_value = 0u;
}

void AnalogMonitor::draw()
{

  display_obj->setTextSize(1);             // Normal 1:1 pixel scale
  display_obj->setTextColor(WHITE);
  display_obj->drawRect(u8_x, u8_y, 115, 6, WHITE);
  display_obj->fillRect(u8_x, u8_y, map(u16_value, 0, 1023, 0, 113), 6, WHITE);
  // display_obj->setCursor(0, u8_y);
  // display_obj->print(c_name);
}

void AnalogMonitor::update(uint16_t value)
{
  u16_value = value;
}


Button::Button(Adafruit_SSD1306* display_obj)
                    :Component(display_obj)
{
  
}


void Button::draw()
{
  //drawrect
  // draw text
  // if is selected invert colors
}

void Button::update(bool input, bool currentlySelected)
{

  b_isSelected = currentlySelected;
  b_isClicked  = input;
}

View::View()
{
  u8_Component_Idx             = 0u;
  u8_Button_Idx                = 0u;
  u8_Currently_Selected_Button = 0u;
}

void View::draw()
{
  uint8_t i;
  for(i = 0; i < u8_Component_Idx; i++)
  {
    e_Components[i]->draw();
  }

}

void View::update(View_t_Input input)
{
  uint8_t i;
  if(input.b_ButtonMoveDown)
  {
    u_AddRoundRobinIdx(u8_Currently_Selected_Button, u8_Button_Idx+1);
  }

  if(input.b_ButtonMoveUp)
  {
    u_SubRoundRobinIdx(u8_Currently_Selected_Button, u8_Button_Idx+1);
  }


  for(i = 0; i < u8_Button_Idx+1; i++)
  {
    e_InputComponents[i]->update(input.b_ButtonClicked, (i == u8_Currently_Selected_Button));
  } 


  for(i = 0; i < u8_Component_Idx; i++)
  {
    e_Components[i]->update(512u);
  }

}

void View::addComponent(Component* component)
{
  e_Components[u8_Component_Idx] = component;
  u8_Component_Idx++;
}

void View::addButton(Button* button)
{
  e_InputComponents[u8_Button_Idx] = button;
  u8_Button_Idx++;
}



// RemoteDisplay::RemoteDisplay()
// {
//   d.setFont(&Picopixel);
// }

// void RemoteDisplay::printBatteryOLED(float battery_percentage)
// {
//   uint8_t fill_width = map(battery_percentage, 0.0f, 100.0f, BATTERY_INDICATION_CHARGE_MIN, BATTERY_INDICATION_CHARGE_MAX);
//   d.fillRect(BATTERY_INDICATION_POSITION_X + BATTERY_INDICATION_WIDTH, 
//                   BATTERY_INDICATION_POSITION_Y + (BATTERY_INDICATION_HEIGHT*0.25f), 
//                   BATTERY_INDICATION_CHARGE_BLANK, (BATTERY_INDICATION_HEIGHT*0.5f), WHITE); // The little rectangle of a battery :)
  
//   d.fillRect(BATTERY_INDICATION_CHARGE_POSITION_X, BATTERY_INDICATION_CHARGE_POSITION_Y, fill_width, BATTERY_INDICATION_CHARGE_HEIGHT, WHITE);
//   d.drawRect(BATTERY_INDICATION_POSITION_X, BATTERY_INDICATION_POSITION_Y, BATTERY_INDICATION_WIDTH, BATTERY_INDICATION_HEIGHT, WHITE);
  
//   d.setFont(&Picopixel);
//   d.setTextSize(1);             // Normal 1:1 pixel scale
//   d.setTextColor(WHITE);
//   d.setCursor(BATTERY_INDICATION_POSITION_X-15, BATTERY_INDICATION_POSITION_Y+5);
//   d.print(battery_percentage, 1);
//   // display.setFont();
// }


// void RemoteDisplay::printConnectionStatusOLED(int tx_time, bool no_connection)
// {
//   static bool b_Blink = false;
//   static uint32_t u32_TimePassed = millis();

 
//   for(uint8_t i = 0; i < N_TICKS; i++)
//   {
//     d.fillRect(COMMUNICATION_TICK_SIZE*i, COMMUNICATION_TICK_SIZE*(N_TICKS-i-1), COMMUNICATION_TICK_WIDTH, COMMUNICATION_TICK_SIZE*i+1, WHITE);
//   }

//   d.setFont(&Picopixel);
//   d.setTextSize(1);             // Normal 1:1 pixel scale
//   d.setTextColor(WHITE);
//   d.setCursor(COMMUNICATION_TICK_WIDTH*5, 5);
//   if(!no_connection) // Only write the tx time if there is connection 
//   {
 
//     d.print((float)(tx_time * 0.001f));
//     d.print(F("ms"));
//   }
//   else // In case no communication, just print No communication and blink with 1sec
//   {
//     if(b_Blink)
//     {
//       d.print(F("No comm!"));
//     }
    
//     if(millis() - u32_TimePassed > 1000)
//     {
//       u32_TimePassed = millis(); // Save current time
//       b_Blink = !b_Blink;      
//     }
//   }

  
// }

