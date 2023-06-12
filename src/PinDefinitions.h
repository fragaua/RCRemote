/**
 * @file PinDefinitions.h
 * @author Marcelo Fraga
 * @brief Arduino pinout definition for the transmitter module of the Ferrari RC Project.
 *        This is project specific. There is a different pinout definition for the receiver part.
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */



#ifndef PINDEFINITIONS_H
#define PINDEFITIONS_H

/* 
 *  PIN Definitions  
 */
// #define JOYSTICK_LEFT_AXIS_X_PIN  A0
// #define JOYSTICK_LEFT_AXIS_Y_PIN  A1
// #define JOYSTICK_LEFT_SWITCH_PIN  7
// #define JOYSTICK_RIGHT_AXIS_X_PIN A2
// #define JOYSTICK_RIGHT_AXIS_Y_PIN A3
// #define JOYSTICK_RIGHT_SWITCH_PIN 6


// #define POT_LEFT_PIN              A6
// #define POT_RIGHT_PIN             A4

// #define SWITCH_SP_PIN             2

// #define BUTTON_LEFT_ZERO_PIN      3
// //#define BUTTON_LEFT_ONE_PIN       4
// #define BUTTON_RIGHT_ZERO_PIN     4
// //#define BUTTON_RIGHT_ONE_PIN      6
// #define LED_LEFT_PIN              5
// #define LED_RIGHT_PIN             A5 // Needs to be A5 because A6 and A7 can only be used as analog in arduino nano

// #define RF24_CSN_PIN              8  // When CSN is low, module listens on SPI port
// #define RF24_CE_PIN               9  // CE Selects whether transmit or receive
// #define RF24_MOSI_PIN             11 // SPI input. Used to receive data from uC
// #define RF24_MISO_PIN             12 // SPI output. Used to send data to uC
// #define RF24_SCK_PIN              13 // Serial clock



/*
    Pin Defenitions V3
*/

#define JOYSTICK_LEFT_AXIS_X_PIN  A2
#define JOYSTICK_LEFT_AXIS_Y_PIN  A3
#define JOYSTICK_LEFT_SWITCH_PIN  6
#define JOYSTICK_RIGHT_AXIS_X_PIN A0
#define JOYSTICK_RIGHT_AXIS_Y_PIN A1
#define JOYSTICK_RIGHT_SWITCH_PIN 7

//A4, A5, A6, A7

//2 -> display
//1 -> battery
//2 -> Pot
//1 -> Buttons -> lets discard and simply read in digital.
#define BUTTON_ANALOG_PIN        A7

#define BATTERY_INDICATION_PIN   A6

#define POT_LEFT_PIN              A5 // Digital 1
#define POT_RIGHT_PIN             A5 // Gitial
#define POT_LEFT_ACTIVATE_PIN     4
#define POT_RIGHT_ACTIVATE_PIN     5

// We can use 1 analog to read both pots.

#define SWITCH_SP_RIGHT_PIN       2
#define SWITCH_SP_LEFT_PIN        3


#define DISPLAY_SCL               A5
#define DISPLAY_SCA               A4



#define RF24_CSN_PIN              8  // When CSN is low, module listens on SPI port
#define RF24_CE_PIN               9  // CE Selects whether transmit or receive
#define RF24_MOSI_PIN             11 // SPI input. Used to receive data from uC
#define RF24_MISO_PIN             12 // SPI output. Used to send data to uC
#define RF24_SCK_PIN              13 // Serial clock


#endif
