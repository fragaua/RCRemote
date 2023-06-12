/**
 * @file RadioPayloadTypes.h
 * @author Marcelo Fraga
 * @brief Transmission/Receiving types for both the transmitter and receiver module. This is shared file between both projects.
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef RADIOPAYLOADTYPES_H
#define RADIOPAYLOADTYPES_H

#include <Joystick_if.h>
#include "PinDefinitions.h"

/*
 * Remote input related definitions and declarations
 */

#define N_BUTTONS 3
// #define BUTTON_LEFT_ZERO_IDX  0
// #define BUTTON_LEFT_ONE_IDX   1
// #define BUTTON_RIGHT_ZERO_IDX 2
// #define BUTTON_RIGHT_ONE_IDX  3

typedef struct JoystickData{
  uint16_t    u16_Value_X;
  uint16_t    u16_Value_Y;
  bool        b_Value_Sw;
}JoystickData;

typedef struct RFPayload{
  JoystickData d_Joystick_Left;
  JoystickData d_Joystick_Right;
  uint16_t     u16_Pot_Left;
  uint16_t     u16_Pot_Right;
  uint8_t      u8_Sw_Left;
  uint8_t      u8_Sw_Right;
  uint8_t      u8_But[N_BUTTONS];
}RFPayload;



#endif
