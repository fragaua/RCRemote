#include <RF24.h>
#include <Joystick_if.h>
#include "Configuration.h"
#include "TypeDefinitions.h"
#include "RemoteDisplay.h"

#define ON  1u
#define OFF 0u

#define DEBUG_BUTTONS ON


#define BATTERY_INDICATION OFF
#define OLED_SCREEN        ON


#define DEBUG_MODE   0 // Debug mode prints all current inputs into serial monitor and has a 1 sec delay. Radio doesn't work in this mode
#define NORMAL_MODE  1 
#define DEBUG_OLED   2

// Define what is the running mode.
#define RUN_MODE DEBUG_MODE


/*
* NRF24L01 RFCom related
*/
#define TX_TIME_LONG    1000 // in us. Time to trigger LED bklinking if tx was too long
#define TX_BLINK_TIME   2000000 // in usecs.  LED blink time
#define TX_CONNECTION_LOST_COUNTER_THRESHOLD 10 // After this ammount of successive failed sends, declare connection lost
#define RF_ADDRESS_SIZE 6
const byte RF_Address[RF_ADDRESS_SIZE] = "1Node";

#define N_CHANNELS 9u 
#define N_BUTTONS  3u 

typedef uint16_t RemoteInputs_t;
typedef uint8_t  InternalRemoteInputs_t; // Buttons aren't considered channels. Are only used for internal use and menu navigation

typedef struct RFPayload{
  uint16_t u16_Channels[N_CHANNELS];
}RFPayload;

#define JOYSTICK_LEFT_AXIS_X_CHANNEL_IDX  0u
#define JOYSTICK_LEFT_AXIS_Y_CHANNEL_IDX  1u
#define JOYSTICK_LEFT_SWITCH_CHANNEL_IDX  2u
#define JOYSTICK_RIGHT_AXIS_X_CHANNEL_IDX 3u
#define JOYSTICK_RIGHT_AXIS_Y_CHANNEL_IDX 4u
#define JOYSTICK_RIGHT_SWITCH_CHANNEL_IDX 5u
#define POT_RIGHT_CHANNEL_IDX             6u
#define SWITCH_SP_RIGHT_CHANNEL_IDX       7u
#define SWITCH_SP_LEFT_CHANNEL_IDX        8u


Input_t RemoteInputs[N_CHANNELS] = {{JOYSTICK_LEFT_AXIS_X_PIN,  0u, true}, {JOYSTICK_LEFT_AXIS_Y_PIN,  0u, true}, {JOYSTICK_LEFT_SWITCH_PIN,  0u, false},
                                    {JOYSTICK_RIGHT_AXIS_X_PIN, 0u, true}, {JOYSTICK_RIGHT_AXIS_Y_PIN, 0u, true}, {JOYSTICK_RIGHT_SWITCH_PIN, 0u, false},
                                    {POT_RIGHT_PIN,             0u, true}, {SWITCH_SP_LEFT_PIN,        0u, false},{SWITCH_SP_RIGHT_PIN,       0u, false}};



InternalRemoteInputs_t InternalRemoteInputs[N_BUTTONS];

#if BATTERY_INDICATION == ON
#include <BatteryIndication.h>
#define R1 10000
#define R2 10000
BatteryIndication battery(BATTERY_INDICATION_PIN, R1, R2, BATTERY_9V);
#endif

#if OLED_SCREEN == ON
RemoteDisplay     display_wrapper;
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

float data = 0.01;
void v_Remote_Modules_Init()
{

  // Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_Sw = JOYSTICK_LEFT_SWITCH_PIN;
  // Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_X  = JOYSTICK_LEFT_AXIS_X_PIN;
  // Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_Y  = JOYSTICK_LEFT_AXIS_Y_PIN;
  // Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_Sw = JOYSTICK_RIGHT_SWITCH_PIN;
  // Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_X  = JOYSTICK_RIGHT_AXIS_X_PIN;
  // Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_Y  = JOYSTICK_RIGHT_AXIS_Y_PIN;
  
  // Transmitter_Remote.Joystick_Left.j_Interface       = Joystick_if(JOYSTICK_LEFT_AXIS_X_PIN, JOYSTICK_LEFT_AXIS_Y_PIN, JOYSTICK_LEFT_SWITCH_PIN); 
  // Transmitter_Remote.Joystick_Right.j_Interface      = Joystick_if(JOYSTICK_RIGHT_AXIS_X_PIN,JOYSTICK_RIGHT_AXIS_Y_PIN, JOYSTICK_RIGHT_SWITCH_PIN); 
  
  // Transmitter_Remote.Pot_Left.u8_Pin                  = POT_LEFT_PIN;
  // Transmitter_Remote.Pot_Right.u8_Pin                 = POT_RIGHT_PIN;
  // Transmitter_Remote.Switch_SPST_Right.u8_Pin         = SWITCH_SP_RIGHT_PIN;
  // Transmitter_Remote.Switch_SPST_Left.u8_Pin          = SWITCH_SP_LEFT_PIN;

  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    pinMode(RemoteInputs[i].u8_Pin,    RemoteInputs[i].b_Analog ? INPUT : INPUT_PULLUP);
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


void v_Compute_Button_Voltage_Dividers(Button *Buttons)
{
  int analog_buttons_read = analogRead(BUTTON_ANALOG_PIN);
  for(int i = 0; i < N_BUTTONS; i++)
  {
    Buttons[i].u8_Value = 0; // Reset buttons by default
  }

  Serial.print("Buts v div: ");
  Serial.println(analog_buttons_read);
  if(analog_buttons_read < 505)
  {
    // Serial.println(F"Button1");
    Buttons[0].u8_Value = 1;
  }
  else if(analog_buttons_read > 505 && analog_buttons_read < 650)
  {
    Buttons[1].u8_Value = 1;
    // Serial.println(F"Button2");
  }
  else if(analog_buttons_read > 650 && analog_buttons_read < 1023)
  {
    Buttons[2].u8_Value = 1;
    // Serial.println(F"Button3");
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
    }
    else
    {
      RemoteInputs[i].u16_Value = map((uint16_t)digitalRead(RemoteInputs[i].u8_Pin), 0, 1, 0, 1023);
    }
  }
}

void v_Build_Payload(Input_t *const p_RemoteInput, RFPayload *p_payload)
{
  uint8_t i;
  for(i = 0; i < N_CHANNELS; i++)
  {
    p_payload->u16_Channels[i] = p_RemoteInput[i].u16_Value;
  }

}


void v_Blink_Led(LED* led, uint32_t usecs, bool activate)
{
  static uint8_t u8_prev_led_value; // Used like this so we overwrite any writting from before
  static uint32_t u32_time_activated;

  if(activate == true)
  {
    u32_time_activated = micros();
  }

  uint32_t delay = micros() - u32_time_activated;
  if(delay < usecs)
  {
    led->u8_Value = !u8_prev_led_value; // Invert output
  }

  u8_prev_led_value = led->u8_Value;
}


#if RUN_MODE == DEBUG_MODE
void printJoystick(Joystick joystick, String joystick_name)
{
  Serial.print("Joystick ");
  Serial.print(joystick_name);
  Serial.print(" (");
  Serial.print(joystick.d_Joystick.u16_Value_X);
  Serial.print(", ");
  Serial.print(joystick.d_Joystick.u16_Value_Y);
  Serial.print(", ");
  Serial.print(joystick.d_Joystick.b_Value_Sw);
}
#endif




void setup() {
  Serial.begin(115200);
  // printf_begin();

  Serial.print(F("Started!\n"));

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();

  v_Remote_Modules_Init();
  Serial.print(freeRam());
  Serial.print(F(" bytes"));
}


void loop() {
  display.clearDisplay();
  static int i32_previous_tx_time;
  static uint8_t u8_not_received_counter = 0;
 
  v_Read_Inputs(RemoteInputs);

  display_wrapper.printJoystickOLED(RemoteInputs[JOYSTICK_LEFT_AXIS_X_CHANNEL_IDX].u16_Value, RemoteInputs[JOYSTICK_LEFT_AXIS_Y_CHANNEL_IDX].u16_Value,
                                    RemoteInputs[JOYSTICK_RIGHT_AXIS_X_CHANNEL_IDX].u16_Value, RemoteInputs[JOYSTICK_RIGHT_AXIS_Y_CHANNEL_IDX].u16_Value);


  // v_Compute_Button_Voltage_Dividers(Buttons);

  v_Build_Payload(RemoteInputs, &payload);

#if BATTERY_INDICATION == ON
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);
#endif

#if RUN_MODE == NORMAL_MODE
  unsigned long start_timer = micros();                
  bool RF_OK = Transmitter_Remote.Radio.write(&payload, sizeof(RFPayload));
  unsigned long end_timer = micros();
  int i32_tx_time = end_timer - start_timer;
  
  // v_SetLEDs_Tx(&(Transmitter_Remote.LEDs[LED_LEFT_IDX]), &(Transmitter_Remote.LEDs[LED_RIGHT_IDX]), RF_OK);
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

    // Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
  }
  
  display_wrapper.printConnectionStatusOLED(i32_tx_time, b_ConnectionLost);

  // v_Write_Led(&(Transmitter_Remote.LEDs[LED_LEFT_IDX]));
  // v_Write_Led(&(Transmitter_Remote.LEDs[LED_RIGHT_IDX]));
  i32_previous_tx_time = i32_tx_time;
  delay(30);
#endif

#if RUN_MODE == DEBUG_OLED
    display.clearDisplay();

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(4);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(F("Hello, world!"));

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.println(3.141592);

    display.setTextSize(2);             // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.print(F("0x")); display.println(0xDEADBEEF, HEX);

    display.display();

    delay(2000);
#endif
// #if RUN_MODE == DEBUG_MODE
#if DEBUG_BATTERY_INDICATION == ON
    Serial.print(F("Battery %: "));
    Serial.println(battery.getBatteryPercentage());
    Serial.println(battery.getCurrentVoltage());
#endif


  display.display(); // Only display the buffer at the end of each loop
}
