#include <RF24.h>
#include <Joystick_if.h>
#include "PinDefinitions.h"
#include "TypeDefinitions.h"
#include <BatteryIndication.h>
#include "RemoteDisplay.h"


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


#define R1 10000
#define R2 10000

BatteryIndication battery(BATTERY_INDICATION_PIN, R1, R2, BATTERY_9V);
RemoteDisplay     display_wrapper;

Remote Transmitter_Remote;
RFPayload payload;

RF24 radio(9, 8);  // using pin 7 for the CE pin, and pin 8 for the CSN pin


bool b_ConnectionLost = false;

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

float data = 0.01;
void v_Remote_Modules_Init()
{
  Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_Sw = JOYSTICK_LEFT_SWITCH_PIN;
  Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_X  = JOYSTICK_LEFT_AXIS_X_PIN;
  Transmitter_Remote.Joystick_Left.u8_Pin_Joystick_Y  = JOYSTICK_LEFT_AXIS_Y_PIN;
  Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_Sw = JOYSTICK_RIGHT_SWITCH_PIN;
  Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_X  = JOYSTICK_RIGHT_AXIS_X_PIN;
  Transmitter_Remote.Joystick_Right.u8_Pin_Joystick_Y  = JOYSTICK_RIGHT_AXIS_Y_PIN;
  
  Transmitter_Remote.Joystick_Left.j_Interface       = Joystick_if(JOYSTICK_LEFT_AXIS_X_PIN, JOYSTICK_LEFT_AXIS_Y_PIN, JOYSTICK_LEFT_SWITCH_PIN); 
  Transmitter_Remote.Joystick_Right.j_Interface      = Joystick_if(JOYSTICK_RIGHT_AXIS_X_PIN,JOYSTICK_RIGHT_AXIS_Y_PIN, JOYSTICK_RIGHT_SWITCH_PIN); 
  
  Transmitter_Remote.Pot_Left.u8_Pin                  = POT_LEFT_PIN;
  Transmitter_Remote.Pot_Right.u8_Pin                 = POT_RIGHT_PIN;
  Transmitter_Remote.Switch_SPST_Right.u8_Pin         = SWITCH_SP_RIGHT_PIN;
  Transmitter_Remote.Switch_SPST_Left.u8_Pin          = SWITCH_SP_LEFT_PIN;

  pinMode(Transmitter_Remote.Pot_Left.u8_Pin,  INPUT);
  pinMode(Transmitter_Remote.Pot_Right.u8_Pin,  INPUT);
  pinMode(Transmitter_Remote.Switch_SPST_Right.u8_Pin,  INPUT_PULLUP);
  pinMode(Transmitter_Remote.Switch_SPST_Left.u8_Pin,  INPUT_PULLUP);

  // for(int i = 0; i < N_BUTTONS; i++)
  // {
  //   Transmitter_Remote.Buttons[i].u8_Pin = (BUTTON_LEFT_ZERO_PIN + i); // Button Left Zero is the first button. All other pins are 1 more as the last
  //   pinMode(Transmitter_Remote.Buttons[i].u8_Pin, INPUT_PULLUP);
  // }
  
  // Buttons are read form analog input, with voltage divider

  // Transmitter_Remote.LEDs[LED_LEFT_IDX].u8_Pin_LED = LED_LEFT_PIN;
  // Transmitter_Remote.LEDs[LED_RIGHT_IDX].u8_Pin_LED = LED_RIGHT_PIN;
  // pinMode(Transmitter_Remote.LEDs[LED_LEFT_IDX].u8_Pin_LED, OUTPUT);
  // pinMode(Transmitter_Remote.LEDs[LED_RIGHT_IDX].u8_Pin_LED, OUTPUT);

  Transmitter_Remote.Radio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
  bool success = Transmitter_Remote.Radio.begin();
  if(success == true){
    Serial.println(F("Initialized radio!"));
    // Transmitter_Remote.Radio.printDetails(); // Turn on TX Mode

  }else{
    Serial.println(F("Failed radio initialization"));
  }
  Transmitter_Remote.Radio.setPALevel(RF24_PA_LOW);
  Transmitter_Remote.Radio.setPayloadSize(sizeof(RFPayload));
  Transmitter_Remote.Radio.openWritingPipe(RF_Address); 
  Transmitter_Remote.Radio.stopListening(); // Turn on TX Mode
}

void v_Read_Joystick(Joystick *p_Joystick)
{
  p_Joystick->d_Joystick.u16_Value_X = (uint16_t)p_Joystick->j_Interface.getJoystickX_Raw();
  p_Joystick->d_Joystick.u16_Value_Y = (uint16_t)p_Joystick->j_Interface.getJoystickY_Raw();
  p_Joystick->d_Joystick.b_Value_Sw  = (bool)p_Joystick->j_Interface.isSwitchPressed();
}

void v_Read_Pot(Pot *p_Pot)
{
  p_Pot->u16_Value = (uint16_t)analogRead(p_Pot->u8_Pin);
}

void v_Read_Button(Button *p_Button)
{
  p_Button->u8_Value = (uint8_t)!digitalRead(p_Button->u8_Pin); 
}

void v_Read_Switch(Switch *p_Switch)
{
  p_Switch->u8_Value = (uint8_t)digitalRead(p_Switch->u8_Pin);
}

void v_Write_Led(LED *p_Led)
{
  digitalWrite(p_Led->u8_Pin_LED, p_Led->u8_Value);
}

void v_Compute_Button_Voltage_Dividers(Button *Buttons)
{
  int analog_buttons_read = analogRead(BUTTON_ANALOG_PIN);
  for(int i = 0; i < N_BUTTONS; i++)
  {
    Buttons[i].u8_Value = 0; // Reset buttons by default
  }

  // Serial.print("Buts v div: ");
  // Serial.println(analog_buttons_read);
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


void v_Build_Payload(const Remote remote, RFPayload *p_payload)
{
  p_payload->d_Joystick_Left.u16_Value_X  = remote.Joystick_Left.d_Joystick.u16_Value_X;
  p_payload->d_Joystick_Left.u16_Value_Y  = remote.Joystick_Left.d_Joystick.u16_Value_Y;
  p_payload->d_Joystick_Left.b_Value_Sw   = remote.Joystick_Left.d_Joystick.b_Value_Sw;

  p_payload->d_Joystick_Right.u16_Value_X = remote.Joystick_Right.d_Joystick.u16_Value_X;
  p_payload->d_Joystick_Right.u16_Value_Y = remote.Joystick_Right.d_Joystick.u16_Value_Y;
  p_payload->d_Joystick_Right.b_Value_Sw  = remote.Joystick_Right.d_Joystick.b_Value_Sw;

  p_payload->u16_Pot_Left                 = remote.Pot_Left.u16_Value;
  p_payload->u16_Pot_Right                = remote.Pot_Right.u16_Value;

  p_payload->u8_Sw_Right                  = remote.Switch_SPST_Right.u8_Value;
  p_payload->u8_Sw_Left                   = remote.Switch_SPST_Left.u8_Value;

  for(int i = 0; i < N_BUTTONS; i++)
  {
    p_payload->u8_But[i] = remote.Buttons[i].u8_Value;
  }

}

// /*
// * Sets the leds with tx information, either ok or not ok
// **/
// void v_SetLEDs_Tx(LED* red_led, LED* green_led, bool tx_ok)
// {
//   Transmitter_Remote.LEDs[LED_LEFT_IDX].u8_Value = 1;
//   Transmitter_Remote.LEDs[LED_RIGHT_IDX].u8_Value = 0;
//   if(tx_ok == true)
//   {
//     Transmitter_Remote.LEDs[LED_LEFT_IDX].u8_Value = 0;
//     Transmitter_Remote.LEDs[LED_RIGHT_IDX].u8_Value = 1;
//   }
// }

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

  v_Read_Joystick(&(Transmitter_Remote.Joystick_Left));
  v_Read_Joystick(&(Transmitter_Remote.Joystick_Right));
  display_wrapper.printJoystickOLED(Transmitter_Remote.Joystick_Left.d_Joystick.u16_Value_X, Transmitter_Remote.Joystick_Left.d_Joystick.u16_Value_Y,
                                    Transmitter_Remote.Joystick_Right.d_Joystick.u16_Value_X, Transmitter_Remote.Joystick_Right.d_Joystick.u16_Value_Y);

  v_Read_Pot(&(Transmitter_Remote.Pot_Left));
  v_Read_Pot(&(Transmitter_Remote.Pot_Right));

  v_Read_Switch(&(Transmitter_Remote.Switch_SPST_Right));
  v_Read_Switch(&(Transmitter_Remote.Switch_SPST_Left));


  v_Compute_Button_Voltage_Dividers(Transmitter_Remote.Buttons);


  v_Build_Payload(Transmitter_Remote, &payload);
  bool battery_ready = battery.readBatteryVoltage(); // This is working but can't be seen with the arduino connected to pc. Otherwise will read the 5v instead of 9
  display_wrapper.printBatteryOLED(battery.getBatteryPercentage());
  // display_wrapper.printBatteryOLED(99.9);


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
#if RUN_MODE == DEBUG_MODE
    Serial.print(F("Battery %: "));
    Serial.println(battery.getBatteryPercentage());
    Serial.println(battery.getCurrentVoltage());
    printJoystick(Transmitter_Remote.Joystick_Left, "Left");
    printJoystick(Transmitter_Remote.Joystick_Right, "Right");

    Serial.println(F("\n"));
    Serial.print("Potentiometer (Left, Right): (");
    Serial.println(Transmitter_Remote.Pot_Left.u16_Value);
    Serial.print(", ");
    Serial.println(Transmitter_Remote.Pot_Right.u16_Value);
    Serial.print(analogRead(POT_RIGHT_PIN));
    Serial.println(F(") "));
    Serial.println();
    Serial.print("Buttons: ");
    for(uint8_t i = 0; i < N_BUTTONS; i++)
    {
      Serial.print("[");
      Serial.print(i);
      Serial.print("]");
      Serial.print(" : ");
      Serial.print((uint8_t)Transmitter_Remote.Buttons[i].u8_Value);
      Serial.print(", ");

      // Transmitter_Remote.LEDs[i].u8_Value = Transmitter_Remote.Buttons[i].u8_Value;
      // v_Write_Led(&(Transmitter_Remote.LEDs[i]));
    }
    Serial.println();
    Serial.println();
    Serial.print("SPST Switch: ");
    Serial.println(Transmitter_Remote.Switch_SPST_Right.u8_Value);
    Serial.println(Transmitter_Remote.Switch_SPST_Left.u8_Value);
    Serial.println(F("#################################################"));
    delay(100);
#endif


  display.display(); // Only display the buffer at the end of each loop
}
