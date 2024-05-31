
#ifndef DISPLAYFUNCTIONS_H
#define DISPLAYFUNCTIONS_H
#include "Configuration.h" // Project specific DisplayFunctions. Not trying to be a module, therefore will import configuration here
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

#define SCREEN_WIDTH   128 // OLED display width, in pixels
#define SCREEN_HEIGHT  64  // OLED display height, in pixels
#define OLED_RESET     -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 

#define INTERNAL_INPUT_CONFIRM_IDX 0u
#define INTERNAL_INPUT_LEFT_IDX    1u
#define INTERNAL_INPUT_RIGHT_IDX   2u

/* View and display types */
typedef struct View_t_Buttons{
  bool b_CurrentlySelected;
  bool b_ClickedThisCycle;
  char c_Name[MAX_CHAR_LEN];
}View_t_Buttons;
typedef bool     InternalRemoteInputs_t; // Buttons aren't considered channels. Are only used for internal use and menu navigation
typedef uint16_t RemoteInputs_t;


void v_updateOptionButtons(const Adafruit_SSD1306* const display, View_t_Buttons* ViewButtons, InternalRemoteInputs_t InternalRemoteInputs[]);
void v_drawAnalogs(const Adafruit_SSD1306* const display, const RemoteChannelInput_t*  eChannels);
void v_printConnectionStatusOLED(const Adafruit_SSD1306* const display, int tx_time, bool no_connection);
void v_drawOptionButtons(const Adafruit_SSD1306* const display, View_t_Buttons ViewButtons[]);



#endif
