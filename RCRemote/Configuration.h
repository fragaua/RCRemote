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

#define BATTERY_INDICATION        OFF
#define OLED_SCREEN               OFF
#define OLED_SCREEN_LOW_MEM_MODE  ON
#define RESPONSIVE_ANALOG_READ    ON


/* 
 *  PIN Definitions  
 */


#define ANALOG_BUTTON_VDIV_THRESHOLD_DOWN 450u
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




#define JOYSTICK_RIGHT_AXIS_X_PIN A1
#define JOYSTICK_RIGHT_AXIS_Y_PIN A0
#define JOYSTICK_RIGHT_SWITCH_PIN 7
#define JOYSTICK_LEFT_AXIS_X_PIN  A3
#define JOYSTICK_LEFT_AXIS_Y_PIN  A2
#define JOYSTICK_LEFT_SWITCH_PIN  6

#define BUTTON_ANALOG_PIN        A6

#if BATTERY_INDICATION == ON
    #define BATTERY_INDICATION_PIN   A6
#endif

#define POT_LEFT_PIN              A7 
#define POT_RIGHT_PIN             A7 
#define POT_LEFT_ACTIVATE_PIN      5
#define POT_RIGHT_ACTIVATE_PIN     4 // Leftovers of trying to read two analog signals in one single analog input. 

// We can use 1 analog to read both pots.

#define SWITCH_SP_RIGHT_PIN       2
#define SWITCH_SP_LEFT_PIN        3


#define DISPLAY_SCL               A5
#define DISPLAY_SDA               A4

#define RF24_CSN_PIN              8  // When CSN is low, module listens on SPI port
#define RF24_CE_PIN               9  // CE Selects whether transmit or receive
#define RF24_MOSI_PIN             11 // SPI input. Used to receive data from uC
#define RF24_MISO_PIN             12 // SPI output. Used to send data to uC
#define RF24_SCK_PIN              13 // Serial clock


/* Input channels and controller input declarations*/ 

#define MAX_NAME_CHAR 3u
#define N_CHANNELS 7u // 17/06 -> Stopped considering joystick buttons 
#define N_ANALOG_CHANNELS 5u
#define N_BUTTONS  3u 
#define N_VIEW_BUTTONS 3u
#define MAX_CHAR_LEN 2u

#define ANALOG_MAX_VALUE 1023
#define ANALOG_MIN_VALUE 0
#define ANALOG_HALF_VALUE 512



/* Radio configuration */ // TODO: Add here other configurations like PA level and data rate
#define TX_TIMEOUT    300 // in milliseconds. Time to trigger "No communication" on screen

/*
* NRF24L01 RFCom related
*/
#define RF_ADDRESS_SIZE 6
const byte RF_Address[RF_ADDRESS_SIZE] = "1Node";


typedef struct RemoteChannelInput_t
{
  uint8_t  u8_Pin;          // Current configured pin on the board.
  uint16_t u16_Value;       // Current, actual converted value.
  uint8_t  u8_Trim;         // Middle point adjustment. 
  uint8_t  u8_MinValueOffset; // Minimum value Difference, for end-point adjustment. Not actual value in order to save memory and avoid u16 type. 
  uint8_t  u8_MaxValueOffset; // Maximum value Difference, for end-point adjustment.
  bool     b_InvertInput;
  bool     b_Analog;        // Analog input or not
  char     c_Name[MAX_NAME_CHAR+1]; // Channel name
}RemoteChannelInput_t;

//
typedef struct RemoteCommunicationState_t
{
  bool           b_ConnectionLost;
  unsigned long  l_TransmissionTime; // In uSeconds
}RemoteCommunicationState_t;

typedef struct RFPayload
{
  uint16_t u16_Channels[N_CHANNELS];
}RFPayload;

#endif
