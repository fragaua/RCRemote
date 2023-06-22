#include <RF24.h>
#include <Joystick_if.h>
#include "Configuration.h"
#include "RemoteDisplay.h"
#include <ResponsiveAnalogRead.h>
// #include <Wire.h>
// #include <Adafruit_SSD1306.h>
// #include <Adafruit_GFX.h>
// #include <Fonts/Picopixel.h>

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

#define ON  1u
#define OFF 0u

#define DEBUG_BUTTONS ON


#define BATTERY_INDICATION        OFF
#define OLED_SCREEN               ON
#define OLED_SCREEN_LOW_MEM_MODE  ON
#define RESPONSIVE_ANALOG_READ    ON

/*
* NRF24L01 RFCom related
*/
#define TX_TIME_LONG    1000 // in us. Time to trigger LED bklinking if tx was too long
#define TX_BLINK_TIME   2000000 // in usecs.  LED blink time
#define TX_CONNECTION_LOST_COUNTER_THRESHOLD 10 // After this ammount of successive failed sends, declare connection lost
#define RF_ADDRESS_SIZE 6
const byte RF_Address[RF_ADDRESS_SIZE] = "1Node";

/* Input channels and controller input declarations*/
#define MAX_NAME_CHAR 3u
#define N_CHANNELS 7u // 17/06 -> Stopped considering joystick buttons 
#define N_ANALOG_CHANNELS 5u
#define N_BUTTONS  3u 
#define N_VIEW_BUTTONS 3u
#define MAX_CHAR_LEN 4u

#define ANALOG_BUTTON_VDIV_THRESHOLD_DOWN 505u
#define ANALOG_BUTTON_VDIV_THRESHOLD_UP   650u

#define JOYSTICK_LEFT_AXIS_X_CHANNEL_IDX  0u
#define JOYSTICK_LEFT_AXIS_Y_CHANNEL_IDX  1u
// #define JOYSTICK_LEFT_SWITCH_CHANNEL_IDX  2u
#define JOYSTICK_RIGHT_AXIS_X_CHANNEL_IDX 2u
#define JOYSTICK_RIGHT_AXIS_Y_CHANNEL_IDX 3u
// #define JOYSTICK_RIGHT_SWITCH_CHANNEL_IDX 5u
#define POT_RIGHT_CHANNEL_IDX             4u
#define SWITCH_SP_RIGHT_CHANNEL_IDX       5u
#define SWITCH_SP_LEFT_CHANNEL_IDX        6u

#define INTERNAL_INPUT_CONFIRM_IDX 0u
#define INTERNAL_INPUT_LEFT_IDX    1u
#define INTERNAL_INPUT_RIGHT_IDX   2u

typedef uint16_t RemoteInputs_t;
typedef bool     InternalRemoteInputs_t; // Buttons aren't considered channels. Are only used for internal use and menu navigation

typedef struct Input_t{
  uint8_t  u8_Pin;
  uint16_t u16_Value;
  bool     b_Analog;
  char     c_Name[MAX_NAME_CHAR+1];
}Input_t;

typedef struct View_t_Buttons{
  bool b_CurrentlySelected;
  bool b_ClickedThisCycle;
  char c_Name[MAX_CHAR_LEN];
}View_t_Buttons;

typedef struct RFPayload{
  uint16_t u16_Channels[N_CHANNELS];
}RFPayload;


Input_t RemoteInputs[N_CHANNELS] = {{JOYSTICK_LEFT_AXIS_X_PIN,  0u, true, "JLX"}, {JOYSTICK_LEFT_AXIS_Y_PIN,  0u, true, "JLY"}, /*{JOYSTICK_LEFT_SWITCH_PIN,  0u, false, "JLB"},*/
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u, true, "JRX"}, {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u, true, "JRY"}, /*{JOYSTICK_RIGHT_SWITCH_PIN, 0u, false, "JRB"},*/
                                    {POT_RIGHT_PIN,             0u, true, "PR"},  {SWITCH_SP_LEFT_PIN,        0u, false, "SWL"},{SWITCH_SP_RIGHT_PIN,       0u, false, "SWR"}};


InternalRemoteInputs_t InternalRemoteInputs[N_BUTTONS];

View_t_Buttons ViewButtons[N_VIEW_BUTTONS] = {{true, false, "Mon"}, {false, false, "Trm"}, {false, false, "Chn"}};

#if RESPONSIVE_ANALOG_READ == ON
ResponsiveAnalogRead ResponsiveAnalogs[N_ANALOG_CHANNELS];
#endif

#if BATTERY_INDICATION == ON
#include <BatteryIndication.h>
#define R1 10000
#define R2 10000
BatteryIndication battery(BATTERY_INDICATION_PIN, R1, R2, BATTERY_9V);
#endif

#if OLED_SCREEN == ON
// RemoteDisplay     display_wrapper;
Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#endif

// Remote Transmitter_Remote;
RFPayload payload;
RF24 Radio;  // using pin 7 for the CE pin, and pin 8 for the CSN pin


bool b_ConnectionLost = false;

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


void v_Remote_Modules_Init()
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pinMode(RemoteInputs[i].u8_Pin,    RemoteInputs[i].b_Analog ? INPUT : INPUT_PULLUP);
#if RESPONSIVE_ANALOG_READ == ON
    ResponsiveAnalogs[i].begin(RemoteInputs[i].u8_Pin, true);
#endif
  }

  pinMode(POT_LEFT_ACTIVATE_PIN,       OUTPUT);
  pinMode(POT_RIGHT_ACTIVATE_PIN,      OUTPUT);
  digitalWrite(POT_RIGHT_ACTIVATE_PIN, HIGH);
  digitalWrite(POT_LEFT_ACTIVATE_PIN,  LOW);
  
  pinMode(BUTTON_ANALOG_PIN,           INPUT);

  // Buttons are read form analog input, with voltage divider

  Radio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
  bool b_Success = Radio.begin();
  if(b_Success == true)
  {
    Serial.println(F("Initialized radio!"));
    Radio.printDetails(); // Turn on TX Mode
  }
  else
  {
    Serial.println(F("Failed radio initialization"));
  }

  Radio.setPALevel(RF24_PA_LOW);
  Radio.setPayloadSize(sizeof(RFPayload));
  Radio.openWritingPipe(RF_Address); 
  Radio.stopListening(); // Turn on TX Mode
}


void v_Compute_Button_Voltage_Dividers(InternalRemoteInputs_t *Buttons)
{
  int i_Analog_Read = analogRead(BUTTON_ANALOG_PIN);
  uint8_t i;
  for(i = 0; i < N_BUTTONS; i++)
  {
    Buttons[i] = false; // Reset buttons by default
  }

  if(i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_DOWN)
  {
    Buttons[0] = true;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_DOWN && i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_UP)
  {
    Buttons[1] = true;
  }
  else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_UP && i_Analog_Read < 1023)
  {
    Buttons[2] = true;
  }

}


void v_Read_Inputs(Input_t *const p_RemoteInput)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    if(RemoteInputs[i].b_Analog)
    {
      RemoteInputs[i].u16_Value = (uint16_t)analogRead(RemoteInputs[i].u8_Pin);

#if RESPONSIVE_ANALOG_READ == ON
        ResponsiveAnalogs[i].update(RemoteInputs[i].u16_Value);
        RemoteInputs[i].u16_Value = ResponsiveAnalogs[i].getValue();
#endif
    }
    else
    {
      RemoteInputs[i].u16_Value = map((uint16_t)digitalRead(RemoteInputs[i].u8_Pin), 0, 1, 0, 1023);
    }
  }
}

void v_Build_Payload(const Input_t * p_RemoteInput, RFPayload *p_payload)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    p_payload->u16_Channels[i] = p_RemoteInput[i].u16_Value;
  }

}


// /* Display functions */
#if OLED_SCREEN_LOW_MEM_MODE == ON
void v_drawAnalogs(const Input_t*  eChannels)
{
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE); 

  for(int i = 0; i < N_CHANNELS; i++)
  {
    uint8_t y = (i*5) + (i*2) + 15;
    display.drawRect(18, y, 108, 6, WHITE);
    display.fillRect(18, y, map(eChannels[i].u16_Value, 0, 1023, 0, 107), 6, WHITE);
    display.setCursor(0, y);
    display.print(eChannels[i].c_Name);

    display.setFont(&Picopixel);
    display.setCursor(SCREEN_WIDTH/2, y+5);
    display.setTextColor(eChannels[i].u16_Value > 500 ? BLACK : WHITE);
    display.print(eChannels[i].u16_Value);
    display.setFont();
    display.setTextColor(WHITE);

  }
}

// Communication
#define N_TICKS 3
#define COMMUNICATION_TICK_SIZE  3 // The size of the smallest communication tick.   .1| As an example, tick is the dot
#define COMMUNICATION_TICK_WIDTH 2
#define COMMUNICATION_TICK_SPACE 4
#define NO_COMMUNICATION_BLINK_TIME 1000 // in ms
void v_printConnectionStatusOLED(int tx_time, bool no_connection)
{
 
  for(uint8_t i = 0; i < N_TICKS; i++)
  {
    display.fillRect(COMMUNICATION_TICK_SIZE*i, COMMUNICATION_TICK_SIZE*(N_TICKS-i-1), COMMUNICATION_TICK_WIDTH, COMMUNICATION_TICK_SIZE*i+1, WHITE);
  }

  // display.setFont(&Picopixel);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);
  display.setCursor(COMMUNICATION_TICK_WIDTH*5, 0);
  if(!no_connection) // Only write the tx time if there is connection 
  {
    display.print((float)(tx_time * 0.001f));
    display.print(F("ms"));
  }
  else // In case no communication, just print No communication and blink with 1sec
  {
    display.print(F("No comm!"));
  }
}

void v_drawOptionButtons(View_t_Buttons ViewButtons[])
{
  display.setFont(&Picopixel);
  for(uint8_t i = 0; i < N_VIEW_BUTTONS; i++)
  {
    display.drawRect(SCREEN_WIDTH/2 + (i*21), 0, 20, 10, WHITE);
    if(ViewButtons[i].b_CurrentlySelected)
    {
      display.fillRect(SCREEN_WIDTH/2 + (i*21), 0, 20, 10, WHITE);
      display.setTextColor(BLACK);
    }
    else
    {
      display.setTextColor(WHITE);
    }
    display.setCursor(SCREEN_WIDTH/2 + (i*21) + 3, 6);
    display.print(ViewButtons[i].c_Name);
  }

  display.setFont();

}

void v_updateOptionButtons(View_t_Buttons* ViewButtons, InternalRemoteInputs_t InternalRemoteInputs[])
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

  // TODO: Detect button rising edges
  // TODO: Fix button VDivs.
}

#else 
View v;
AnalogMonitor analog_monitors[N_CHANNELS];
#endif



void setup() {
  Serial.begin(115200);
  // printf_begin();

  Serial.println(freeRam());
  Serial.print(F("Bytes\n"));


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }


#if OLED_SCREEN_LOW_MEM_MODE == OFF
  v = View();
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    analog_monitors[i] = AnalogMonitor(&display);
    analog_monitors[i].setPosition(18, (i*5) + (i*2) + 15);
    v.addComponent(&(analog_monitors[i]));
  }
#endif

  v_Remote_Modules_Init();
  display.display();
}


void loop() {
  display.clearDisplay();
  static int i32_previous_tx_time;
  static uint8_t u8_not_received_counter = 0;
  

  v_Read_Inputs(RemoteInputs);

  v_Compute_Button_Voltage_Dividers(InternalRemoteInputs);

  v_Build_Payload(RemoteInputs, &payload);

#if BATTERY_INDICATION == ON
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);
#endif

  unsigned long start_timer = micros();                
  bool RF_OK = Radio.write(&payload, sizeof(RFPayload));
  unsigned long end_timer = micros();
  int i32_tx_time = end_timer - start_timer;
  
  if (RF_OK) 
  {
    u8_not_received_counter = 0; // Reset not received counter
    b_ConnectionLost = false; // TODO: Check if it's enough to just set as true once we send one message

    
    if((i32_tx_time - i32_previous_tx_time) > TX_TIME_LONG)
    {
   
 
    }
 
  } 
  else 
  {
    if(u8_not_received_counter >= TX_CONNECTION_LOST_COUNTER_THRESHOLD)
    {
      b_ConnectionLost = true;
    }
    else
    {
      u8_not_received_counter++;
    }

  }
  
#if OLED_SCREEN_LOW_MEM_MODE == ON
  v_updateOptionButtons(ViewButtons, InternalRemoteInputs);
  v_drawOptionButtons(ViewButtons);
  v_drawAnalogs(RemoteInputs);
  v_printConnectionStatusOLED(i32_tx_time, b_ConnectionLost);
#else
  View_t_Input inputs = {InternalRemoteInputs[0], InternalRemoteInputs[1], InternalRemoteInputs[2]};
  v.update(inputs);
  v.draw();
#endif

  i32_previous_tx_time = i32_tx_time;

#if DEBUG_BATTERY_INDICATION == ON
    Serial.print(F("Battery %: "));
    Serial.println(battery.getBatteryPercentage());
    Serial.println(battery.getCurrentVoltage());
#endif


  display.display(); // Only display the buffer at the end of each loop
  delay(5);
}
