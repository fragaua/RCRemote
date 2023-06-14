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

/* 
 *  PIN Definitions  
 */

#define JOYSTICK_RIGHT_AXIS_X_PIN A0
#define JOYSTICK_RIGHT_AXIS_Y_PIN A1
#define JOYSTICK_RIGHT_SWITCH_PIN 7
#define JOYSTICK_LEFT_AXIS_X_PIN  A2
#define JOYSTICK_LEFT_AXIS_Y_PIN  A3
#define JOYSTICK_LEFT_SWITCH_PIN  6

#define BUTTON_ANALOG_PIN        A6

#if BATTERY_INDICATION == ON
    #define BATTERY_INDICATION_PIN   A6
#endif

#define POT_LEFT_PIN              A7 
#define POT_RIGHT_PIN             A7 
#define POT_LEFT_ACTIVATE_PIN      5
#define POT_RIGHT_ACTIVATE_PIN     4

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
