/**
 * @file Configuration.h
 * @author Marcelo Fraga
 * @brief Arduino pinout definition for the transmitter module of the RC Transmitter.
 *        This is project specific. There is a different pinout definition for the receiver part.
 *        It also contains a group of configuration for activating and de-activating different modules like the OLED or battery indication
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */



#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <Arduino.h>

#define ON  1u
#define OFF 0u


/*
*
*   Feature availability
*
*/


#define DEBUG                     OFF

#define BATTERY_INDICATION        OFF
#define OLED_SCREEN               OFF
#define OLED_SCREEN_LOW_MEM_MODE  ON
#define RESPONSIVE_ANALOG_READ    OFF // TODO: Make sure we can disable responsive read. At this point it isn't possible without breaking the software.
#define TIMEOUT_DETECTION         OFF

/* 
 *  Channel configuration indices  
*/

// Input Buttons Voltage divider thresholds
#define ANALOG_BUTTON_VDIV_THRESHOLD_B1   450u
#define ANALOG_BUTTON_VDIV_THRESHOLD_B2   650u
#define ANALOG_BUTTON_VDIV_THRESHOLD_B3   950u

#define JOYSTICK_LEFT_AXIS_X_CHANNEL_IDX  0u
#define JOYSTICK_LEFT_AXIS_Y_CHANNEL_IDX  1u
#define JOYSTICK_LEFT_SWITCH_CHANNEL_IDX  2u

#define JOYSTICK_RIGHT_AXIS_X_CHANNEL_IDX 2u
#define JOYSTICK_RIGHT_AXIS_Y_CHANNEL_IDX 3u
#define JOYSTICK_RIGHT_SWITCH_CHANNEL_IDX 5u

#define POT_LEFT_CHANNEL_IDX              4u
#define POT_RIGHT_CHANNEL_IDX             5u
#define SWITCH_SP_LEFT_CHANNEL_IDX        6u
#define SWITCH_SP_RIGHT_CHANNEL_IDX       7u



/* 
 *  PIN Definitions  
*/

#define JOYSTICK_RIGHT_AXIS_X_PIN A1
#define JOYSTICK_RIGHT_AXIS_Y_PIN A0
#define JOYSTICK_RIGHT_SWITCH_PIN 7
#define JOYSTICK_LEFT_AXIS_X_PIN  A3
#define JOYSTICK_LEFT_AXIS_Y_PIN  A2
#define JOYSTICK_LEFT_SWITCH_PIN  6

#define BUTTON_ANALOG_PIN        A6 // Not used after wiring re-done

#if BATTERY_INDICATION == ON
    #define BATTERY_INDICATION_PIN   A6
#endif

#define POT_LEFT_PIN              A7 
#define POT_RIGHT_PIN             A6 
#define POT_LEFT_ACTIVATE_PIN      5
#define POT_RIGHT_ACTIVATE_PIN     4 // Leftovers of trying to read two analog signals in one single analog input. 

// We can use 1 analog to read both pots.

#define SWITCH_SP_RIGHT_PIN       2
#define SWITCH_SP_LEFT_PIN        3 

#define INPUT_BUTTON_RIGHT_PIN    4
#define INPUT_BUTTON_SELECT_PIN   5
#define INPUT_BUTTON_LEFT_PIN     6


#define DISPLAY_SCL               A5
#define DISPLAY_SDA               A4

#define RF24_CSN_PIN              8  // When CSN is low, module listens on SPI port
#define RF24_CE_PIN               9  // CE Selects whether transmit or receive
#define RF24_MOSI_PIN             11 // SPI input. Used to receive data from uC
#define RF24_MISO_PIN             12 // SPI output. Used to send data to uC
#define RF24_SCK_PIN              13 // Serial clock


/* Input channels and controller input declarations */ 

#define MAX_NAME_CHAR 3u
// 17/06 -> Stopped considering joystick buttons
// 24/07/2024 -> Re-done wiring on a physical level, allowing for more channels
// NOTE: Changing this requires that you flash new software on the receiver, with the same nr of channels
// This is due to thte fact that this macro is used on the payload definition as well.
#define N_CHANNELS 8u  
#define N_ANALOG_CHANNELS 6u
#define N_BUTTONS  3u 

#define ANALOG_MAX_VALUE 1023
#define ANALOG_MIN_VALUE 0
#define ANALOG_HALF_VALUE 512



#define EXPONENTIAL_VALUE 0.9f


/* Radio configuration */ // TODO: Add here other configurations like PA level and data rate
#define TX_TIMEOUT    5000 // in milliseconds. Time to trigger "No communication" on screen

/*
* NRF24L01 RFCom related
*/
#define RF_ADDRESS_SIZE 3u
const byte RF_Address[RF_ADDRESS_SIZE] = "FG";


typedef struct RemoteChannelInput_t
{
  uint8_t  u8_Pin;              // Current configured pin on the board.
  uint16_t u16_Value;           // Current, actual converted value.
  uint16_t u16_RawValue;        // Current actual raw value without trimming or endpoint adjustment
  uint16_t u16_Trim;            // Middle point absolute value. Offset is computed in the processTrimming function.
  uint16_t u16_MinValue;        // Absolute value for MinValue limit, for end point adjustment
  uint16_t u16_MaxValue;        // Absolute value for MaxValue limit, for end point adjustment
  bool     b_InvertInput;
  bool     b_Analog;            // Analog input or not
  bool     b_expControl;        // Exponential control (For now, while in this low memory controller, don't allow for tuning of this)
  char     c_Name[MAX_NAME_CHAR+1]; // Channel name
}RemoteChannelInput_t;
// TODO: on higher memory controllers, we should have a raw and processed value saved in this structure instead of changing the actual value.

//
typedef struct RemoteCommunicationState_t
{
  bool           b_ConnectionLost;
  unsigned long  l_TransmissionTime; // In uSeconds
}RemoteCommunicationState_t;

// Radio interface. Changes in this structure involve changes on the receiver as well
typedef struct RFPayload
{
  uint16_t u16_Channels[N_CHANNELS];
}RFPayload;

#endif
