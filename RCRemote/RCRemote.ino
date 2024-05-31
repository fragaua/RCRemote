#include "Configuration.h"
#include <RF24.h>
#include <Joystick_if.h>

#include <U8g2lib.h>
#include <Wire.h>

#if OLED_SCREEN == ON
  #if OLED_SCREEN_LOW_MEM_MODE == ON
    #include "DisplayFunctions.h"
  #else
    #include "RemoteDisplay.h"
  #endif
#endif

typedef U8G2_SSD1306_128X64_NONAME_1_SW_I2C DisplayHandler;

#define DEBUG_BUTTONS ON


/*
* NRF24L01 RFCom related
*/
#define RF_ADDRESS_SIZE 6
const byte RF_Address[RF_ADDRESS_SIZE] = "1Node";


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



typedef struct RFPayload{
  uint16_t u16_Channels[N_CHANNELS];
}RFPayload;


// Declare and configure each input on the remote controller.
// TODO: Later, the menus and inputs should be used to configure trimming and end-point adjustment on the fly.
// They shouldn't be configured here like some of the inputs are.
                                   // Pin, Value, Trim, Min, Max, isAnalog, Invert, Channel Name  
RemoteChannelInput_t RemoteInputs[N_CHANNELS] = {{JOYSTICK_LEFT_AXIS_X_PIN,  0u, 0u, 0u,   0u,   false, true, "JLX"}, 
                                    {JOYSTICK_LEFT_AXIS_Y_PIN,  0u, 0u, 255u, 255u, false, true, "JLY"}, 
                                    /*{JOYSTICK_LEFT_SWITCH_PIN,  0u, false, "JLB"},*/
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u, 0u, 255u, 255u, true,  true, "JRX"}, 
                                    {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u, 0u, 0u,   0u,   false, true, "JRY"}, 
                                    /*{JOYSTICK_RIGHT_SWITCH_PIN, 0u, false, "JRB"},*/
                                    {POT_RIGHT_PIN,             0u, 0u, 0u,   0u,   false, true, "PR"},  
                                    {SWITCH_SP_LEFT_PIN,        0u, 0u, 0u,   0u,   false, false, "SWL"}, 
                                    {SWITCH_SP_RIGHT_PIN,       0u, 0u, 0u,   0u,   false, false, "SWR"}};




#if RESPONSIVE_ANALOG_READ == ON
#include <ResponsiveAnalogRead.h>
ResponsiveAnalogRead ResponsiveAnalogs[N_ANALOG_CHANNELS];
#endif

#if BATTERY_INDICATION == ON
#include <BatteryIndication.h>
#define R1 10000
#define R2 10000
BatteryIndication battery(BATTERY_INDICATION_PIN, R1, R2, BATTERY_9V);
#endif

// Remote Transmitter_Remote;
RFPayload payload;
RF24 Radio;
DisplayHandler display = DisplayHandler(U8G2_R0, DISPLAY_SCL, DISPLAY_SDA, U8X8_PIN_NONE);  



int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


// TODO: make a struct containing the responsive analog reads so that we can easily enable or disable it via if directives
// TODO: Improve naming to enforce the concept of "remote CHANNEL input" and ordinary "remote input" (such as buttons)
void v_initRemoteInputs(RemoteChannelInput_t* pRemoteInputs, ResponsiveAnalogRead* pRespAnalogRead)
{
  // Remote CHANNEL inputs
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pinMode(RemoteInputs[i].u8_Pin,    RemoteInputs[i].b_Analog ? INPUT : INPUT_PULLUP);
#if RESPONSIVE_ANALOG_READ == ON
    ResponsiveAnalogs[i].begin(RemoteInputs[i].u8_Pin, true);
#endif
  }
  // Remote input 
  pinMode(BUTTON_ANALOG_PIN, INPUT);
}

boolean b_initRadio(RF24* pRadio)
{
  *pRadio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
  bool b_Success = pRadio->begin();

  if(b_Success)
  {
    Radio.setPALevel(RF24_PA_LOW);
    Radio.setPayloadSize(sizeof(RFPayload));
    Radio.openWritingPipe(RF_Address); 
    Radio.stopListening(); // Turn on TX Mode
  }
  return b_Success;
}

void v_initDisplay(DisplayHandler* pDisplay)
{
  pDisplay->begin();
  pDisplay->setFont(u8g2_font_Georgia7px_tf);

}


// void v_Compute_Button_Voltage_Dividers(InternalRemoteInputs_t *Buttons)
// {
//   // TODO: Debounce button input
//   int i_Analog_Read = analogRead(BUTTON_ANALOG_PIN);
//   uint8_t i;
//   for(i = 0; i < N_BUTTONS; i++)
//   {
//     Buttons[i] = false; // Reset buttons by default
//   }

//   if(i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_DOWN)
//   {
//     Buttons[0] = true;
//   }
//   else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_DOWN && i_Analog_Read < ANALOG_BUTTON_VDIV_THRESHOLD_UP)
//   {
//     Buttons[1] = true;
//   }
//   else if(i_Analog_Read > ANALOG_BUTTON_VDIV_THRESHOLD_UP && i_Analog_Read < 1023)
//   {
//     Buttons[2] = true;
//   }

// }


void v_readChannelInputs(RemoteChannelInput_t *const pRemoteChannelInput, ResponsiveAnalogRead* pRespAnalogRead)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    if(pRemoteChannelInput[i].b_Analog)
    {
      pRemoteChannelInput[i].u16_Value = (uint16_t)analogRead(pRemoteChannelInput[i].u8_Pin);
      v_invertInput(&pRemoteChannelInput[i]);
      v_processTrimming(&pRemoteChannelInput[i]); // Trimming is processed before adjustment to ensure trim offset doesn't overload the min-max values
      v_processEndpointAdjustment(&pRemoteChannelInput[i]);
#if RESPONSIVE_ANALOG_READ == ON
      pRespAnalogRead[i].update(pRemoteChannelInput[i].u16_Value);
      pRemoteChannelInput[i].u16_Value = pRespAnalogRead[i].getValue();
#endif
    }
    else
    {
      pRemoteChannelInput[i].u16_Value = map((uint16_t)digitalRead(pRemoteChannelInput[i].u8_Pin), LOW, HIGH, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
    }
  }
}

/* Processes endpoint adjustment and overrides provided value if value is outside current configured endpoints */
void v_processEndpointAdjustment(RemoteChannelInput_t* pInput)
{
  uint16_t u16_MaxValue = (ANALOG_MAX_VALUE - pInput->u8_MaxValueOffset);
  uint16_t u16_MinValue = (ANALOG_MIN_VALUE + pInput->u8_MinValueOffset);
  pInput->u16_Value = (pInput->u16_Value > u16_MaxValue) ? u16_MaxValue : pInput->u16_Value; 
  pInput->u16_Value = (pInput->u16_Value < u16_MinValue) ? u16_MinValue : pInput->u16_Value; 
}

/* Process trimming and add the current trim offset to the actual value.*/
void v_processTrimming(RemoteChannelInput_t* pInput)
{
  pInput->u16_Value += pInput->u8_Trim;
}

void v_invertInput(RemoteChannelInput_t* pInput)
{
  if(pInput->b_InvertInput)
  {
    pInput->u16_Value = map(pInput->u16_Value, ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, ANALOG_MAX_VALUE, ANALOG_MIN_VALUE);
  }
}

void v_buildPayload(const RemoteChannelInput_t* pRemoteChannelInput, RFPayload* pPayload)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pPayload->u16_Channels[i] = pRemoteChannelInput[i].u16_Value;
  }

}


// /* Display functions */
#if OLED_SCREEN == ON
  View_t_Buttons              ViewButtons[N_VIEW_BUTTONS] = {{true, false, "Mon"}, {false, false, "Trm"}, {false, false, "Chn"}};
  InternalRemoteInputs_t      InternalRemoteInputs[N_BUTTONS];
  // old high mem mode
  View v;
  AnalogMonitor analog_monitors[N_CHANNELS];
#endif

void setup() {
  Serial.begin(115200);

  v_initDisplay(&display);
  v_initRemoteInputs(RemoteInputs, ResponsiveAnalogs);
  boolean b_initRadioSuccess = b_initRadio(&Radio);
  // TODO: Display a msg on screen if radio wasn't properly initialized

// TODO: See if we can use class based displays with new low mem page buffer
#if OLED_SCREEN == ON
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
#endif
  Serial.println(freeRam()); // TODO: Halt program, use u8x8 instead and display a msg on the screen
  Serial.print(F("Bytes\n"));
}


void loop() {
  display.firstPage();
#if OLED_SCREEN == ON
  display.clearDisplay();
#endif
  // static int i32_previous_tx_time;
  // static uint8_t u8_not_received_counter = 0;
  // bool b_ConnectionLost = false;
  
  // v_readChannelInputs(RemoteInputs);
  // v_Compute_Button_Voltage_Dividers(InternalRemoteInputs);
  // v_buildPayload(RemoteInputs, &payload);

#if BATTERY_INDICATION == ON
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);
#endif

  // unsigned long start_timer = micros();                
  // bool RF_OK = Radio.write(&payload, sizeof(RFPayload));
  // unsigned long end_timer = micros();
  // int i32_tx_time = end_timer - start_timer;
  
  // if (RF_OK) 
  // {
  //   u8_not_received_counter = 0; // Reset not received counter
  //   b_ConnectionLost = false; // TODO: Check if it's enough to just set as true once we send one message

    
  //   if((i32_tx_time - i32_previous_tx_time) > TX_TIME_LONG)
  //   {
   
 
  //   }
 
  // } 
  // else 
  // {
  //   if(u8_not_received_counter >= TX_CONNECTION_LOST_COUNTER_THRESHOLD)
  //   {
  //     b_ConnectionLost = true;
  //   }
  //   else
  //   {
  //     u8_not_received_counter++;
  //   }

  // }
  // i32_previous_tx_time = i32_tx_time;

#if OLED_SCREEN == ON
  #if OLED_SCREEN_LOW_MEM_MODE == ON
    v_updateOptionButtons(&display, ViewButtons, InternalRemoteInputs);
    v_drawOptionButtons(&display, ViewButtons);
    v_drawAnalogs(&display, RemoteInputs);
    v_printConnectionStatusOLED(&display, i32_tx_time, b_ConnectionLost);
  #else
    View_t_Input inputs = {InternalRemoteInputs[0], InternalRemoteInputs[1], InternalRemoteInputs[2]};
    v.update(inputs);
    v.draw();
  #endif
 
  display.display(); // Only display the buffer at the end of each loop
#endif


#if DEBUG_BATTERY_INDICATION == ON
    Serial.print(F("Battery %: "));
    Serial.println(battery.getBatteryPercentage());
    Serial.println(battery.getCurrentVoltage());
#endif

  do
  {
    display.setCursor(0, 20);
    display.print(F("Free Ram: "));  
    display.print(freeRam());  
  }while(display.nextPage());
  // u8g2.sendBuffer();
  delay(20);
}
