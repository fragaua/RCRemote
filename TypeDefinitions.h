/**
 * @file TypeDefinitions.h
 * @author Marcelo Fraga
 * @brief Contains all the type definictions used for the transmitter module.
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TYPEDEFINITIONS_H
#define TYPEDEFINITIONS_H

#include <Joystick_if.h>
#include "PinDefinitions.h"
#include "RadioPayloadTypes.h"

/*
 * Remote input related definitions and declarations
 */

#define N_LEDS                2
#define N_BUTTONS             3
#define LED_LEFT_IDX          0
#define LED_RIGHT_IDX         1
#define BUTTON_LEFT_ZERO_IDX  0
#define BUTTON_LEFT_ONE_IDX   1
#define BUTTON_RIGHT_ZERO_IDX 2
#define BUTTON_RIGHT_ONE_IDX  3


typedef struct Pot{
  uint8_t     u8_Pin;
  uint16_t u16_Value;
}Pot;

typedef struct Switch{
  uint8_t      u8_Pin;
  uint8_t      u8_Value;
}Switch;

typedef struct Button{
  uint8_t      u8_Pin;
  uint8_t      u8_Value;
}Button;

typedef struct Joystick{
  uint8_t      u8_Pin_Joystick_X;
  uint8_t      u8_Pin_Joystick_Y;
  uint8_t      u8_Pin_Joystick_Sw;
  JoystickData d_Joystick;
  Joystick_if  j_Interface;
}Joystick;

typedef struct LED{
  uint8_t u8_Pin_LED;
  uint8_t u8_Value; // Value to be written in LED
}LED;

typedef struct Remote{
  // Inputs
  Joystick Joystick_Left;
  Joystick Joystick_Right;
  Pot      Pot_Left;
  Pot      Pot_Right;
  Switch   Switch_SPST_Right;
  Switch   Switch_SPST_Left;
  Button   Buttons[N_BUTTONS];

  // LED      LEDs[N_LEDS];

  // Receiver Radio Module
  RF24     Radio;
}Remote;



#endif