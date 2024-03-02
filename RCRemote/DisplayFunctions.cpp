#include "DisplayFunctions.h"


void v_updateOptionButtons(const Adafruit_SSD1306* const display, View_t_Buttons* ViewButtons, InternalRemoteInputs_t InternalRemoteInputs[])
{
  uint8_t u8_CurrentlySelectedButton = 0;
  for(uint8_t i = 0; i < N_VIEW_BUTTONS; i++)
  {
    u8_CurrentlySelectedButton = (ViewButtons[i].b_CurrentlySelected) ? i : u8_CurrentlySelectedButton;
  }

  ViewButtons[u8_CurrentlySelectedButton].b_ClickedThisCycle = (ViewButtons[u8_CurrentlySelectedButton].b_CurrentlySelected) && (InternalRemoteInputs[INTERNAL_INPUT_CONFIRM_IDX]);
 
  if(InternalRemoteInputs[INTERNAL_INPUT_LEFT_IDX])
  {
    ViewButtons[u8_CurrentlySelectedButton].b_CurrentlySelected = false;
    ViewButtons[(u8_CurrentlySelectedButton+1) % N_VIEW_BUTTONS].b_CurrentlySelected = true;
  }

  // TODO: Detect button rising edges, debounce input
  // TODO: Fix button VDivs.
}

void v_drawAnalogs(const Adafruit_SSD1306* const display, const Input_t*  eChannels)
{
  display->setTextSize(1);             // Normal 1:1 pixel scale
  display->setTextColor(WHITE); 

  for(int i = 0; i < N_CHANNELS; i++)
  {
    uint8_t y = (i*5) + (i*2) + 15;
    display->drawRect(18, y, 108, 6, WHITE);
    display->fillRect(18, y, map(eChannels[i].u16_Value, 0, 1023, 0, 107), 6, WHITE);
    display->setCursor(0, y);
    display->print(eChannels[i].c_Name);

    display->setFont(&Picopixel);
    display->setCursor(SCREEN_WIDTH/2, y+5);
    display->setTextColor(eChannels[i].u16_Value > 500 ? BLACK : WHITE);
    display->print(eChannels[i].u16_Value);
    display->setFont();
    display->setTextColor(WHITE);

  }
}

// Communication
#define N_TICKS 3
#define COMMUNICATION_TICK_SIZE  3 // The size of the smallest communication tick.   .1| As an example, tick is the dot
#define COMMUNICATION_TICK_WIDTH 2
#define COMMUNICATION_TICK_SPACE 4
#define NO_COMMUNICATION_BLINK_TIME 1000 // in ms
void v_printConnectionStatusOLED(const Adafruit_SSD1306* const display, int tx_time, bool no_connection)
{
 
  for(uint8_t i = 0; i < N_TICKS; i++)
  {
    display->fillRect(COMMUNICATION_TICK_SIZE*i, COMMUNICATION_TICK_SIZE*(N_TICKS-i-1), COMMUNICATION_TICK_WIDTH, COMMUNICATION_TICK_SIZE*i+1, WHITE);
  }

  // display->setFont(&Picopixel);
  display->setTextSize(1);             // Normal 1:1 pixel scale
  display->setTextColor(WHITE);
  display->setCursor(COMMUNICATION_TICK_WIDTH*5, 0);
  if(!no_connection) // Only write the tx time if there is connection 
  {
    display->print((float)(tx_time * 0.001f));
    display->print(F("ms"));
  }
  else // In case no communication, just print No communication and blink with 1sec
  {
    display->print(F("No comm!"));
  }
}

void v_drawOptionButtons(const Adafruit_SSD1306* const display, View_t_Buttons ViewButtons[])
{
  display->setFont(&Picopixel);
  for(uint8_t i = 0; i < N_VIEW_BUTTONS; i++)
  {
    display->drawRect(SCREEN_WIDTH/2 + (i*21), 0, 20, 10, WHITE);
    if(ViewButtons[i].b_CurrentlySelected)
    {
      display->fillRect(SCREEN_WIDTH/2 + (i*21), 0, 20, 10, WHITE);
      display->setTextColor(BLACK);
    }
    else
    {
      display->setTextColor(WHITE);
    }
    display->setCursor(SCREEN_WIDTH/2 + (i*21) + 3, 6);
    display->print(ViewButtons[i].c_Name);
  }

  display->setFont();

}
