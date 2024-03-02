#include "Configuration.h"
#include <RF24.h>
#include <Joystick_if.h>

#if OLED_SCREEN == ON
  #if OLED_SCREEN_LOW_MEM_MODE == ON
    #include "DisplayFunctions.h"
  #else
    #include "RemoteDisplay.h"
  #endif
#endif



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
                                   // Pin, Value, Trim, Min, Max, isAnalog, Channel Name  
Input_t RemoteInputs[N_CHANNELS] = {{JOYSTICK_LEFT_AXIS_X_PIN,  0u, 0u, 0u,   0u,   true, "JLX"}, 
                                    {JOYSTICK_LEFT_AXIS_Y_PIN,  0u, 0u, 0u,   0u,   true, "JLY"}, 
                                    /*{JOYSTICK_LEFT_SWITCH_PIN,  0u, false, "JLB"},*/
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u, 0u, 255u, 255u, true, "JRX"}, 
                                    {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u, 0u, 0u,   0u,   true, "JRY"}, 
                                    /*{JOYSTICK_RIGHT_SWITCH_PIN, 0u, false, "JRB"},*/
                                    {POT_RIGHT_PIN,             0u, 0u, 0u,   0u,   true, "PR"},  
                                    {SWITCH_SP_LEFT_PIN,        0u, 0u, 0u,   0u,   false, "SWL"}, 
                                    {SWITCH_SP_RIGHT_PIN,       0u, 0u, 0u,   0u,   false, "SWR"}};




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
RF24 Radio;  // using pin 7 for the CE pin, and pin 8 for the CSN pin



int freeRam () 
{
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
    //Radio.printDetails();
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
  // TODO: Debounce button input
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
      v_Process_Trimming(&RemoteInputs[i]); // Trimming is processed before adjustment to ensure trim offset doesn't overload the min-max values
      v_Process_Endpoint_Adjustment(&RemoteInputs[i]);
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

/* Processes endpoint adjustment and overrides provided value if value is outside current configured endpoints */
void v_Process_Endpoint_Adjustment(Input_t* p_input)
{
  uint16_t u16_MaxValue = (ANALOG_MAX_VALUE - p_input->u8_MaxValueOffset);
  uint16_t u16_MinValue = (ANALOG_MIN_VALUE + p_input->u8_MinValueOffset);
  p_input->u16_Value = (p_input->u16_Value > u16_MaxValue) ? u16_MaxValue : p_input->u16_Value; 
  p_input->u16_Value = (p_input->u16_Value < u16_MinValue) ? u16_MinValue : p_input->u16_Value; 
}
/* Process trimming and add the current trim offset to the actual value.*/
void v_Process_Trimming(Input_t* p_input)
{
  p_input->u16_Value += p_input->u8_Trim;
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
#if OLED_SCREEN == ON
  View_t_Buttons              ViewButtons[N_VIEW_BUTTONS] = {{true, false, "Mon"}, {false, false, "Trm"}, {false, false, "Chn"}};
  InternalRemoteInputs_t      InternalRemoteInputs[N_BUTTONS];
  #if OLED_SCREEN_LOW_MEM_MODE == ON
   Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 
  #else
    View v;
    AnalogMonitor analog_monitors[N_CHANNELS];
  #endif
#endif

void setup() {
  Serial.begin(115200);
  // printf_begin();

  Serial.println(freeRam()); // TODO: Give visual feedback if ram isn't enough
  Serial.print(F("Bytes\n"));


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
#if OLED_SCREEN == ON
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever // TODO: Give visual feedback (internal LED for example)
  }
#endif

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
  
  display.display();
#endif

  v_Remote_Modules_Init();
}


void loop() {
#if OLED_SCREEN == ON
  display.clearDisplay();
#endif
  static int i32_previous_tx_time;
  static uint8_t u8_not_received_counter = 0;
  bool b_ConnectionLost = false;
  
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

  i32_previous_tx_time = i32_tx_time;

#if DEBUG_BATTERY_INDICATION == ON
    Serial.print(F("Battery %: "));
    Serial.println(battery.getBatteryPercentage());
    Serial.println(battery.getCurrentVoltage());
#endif

  delay(20);
}
