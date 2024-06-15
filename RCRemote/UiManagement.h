/**
 * @file UiManagement.h 
 * @author Marcelo Fraga
 * @brief Header file for UiManagement. This is a project specific component where the UiCore
 * is initialized and all pages and components are instantiated and managed. Inputs are passed
 * down from the original application (or any other module for that matter) in order to make changes
 * to the current UiCore Context manager.
 * 
 * While this particular unit isn't generic, it should be easy to adapt to any project specific scenario.
 * For instance, for each project/Ui we should provide the Receiver and Provider ports. For this remote transmitter
 * I want to receive the remote Channel values to update the analog monitors, as well as the button inputs to manage
 * the Ui (The structure of this latter is defined here in this .h). As a provide port, we are sending the new output
 * configuration that we wish to send (trimming, endpoint etc.)
 * 
 * The init and update functions are update accordingly and all pages and components should be declared in the UiManagement.c
 * 
 * @version 0.1
 * @date 2024 - 06 - 14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef UIMANAGEMENT_H
#define UIMANAGEMENT_H
#include "UiCoreFramework.h"
#include "Configuration.h"




// Contains all the inputs the application is able to handle for User Interaction
typedef struct UiM_t_Inputs
{
    // Project specific set of Ui interface
    bool     inputButtonLeft   : 1;
    bool     inputButtonRight  : 1;
    bool     inputButtonSelect : 1;

    uint16_t scrollWheel; 

}UiM_t_Inputs;


// Contains all the inputs necessary to update the internal components.
typedef struct UiM_t_contextManager
{
    /**** Receiver Ports *****/
    // Project specific data to display
    RemoteChannelInput_t* remoteChannelInputs; // TODO: change this to a "rPort" structure to get also comm data...
    
    // Project specific Ui management input data to manage
    UiM_t_Inputs*         uiManagementInputs;


    /**** Provider Ports *****/
    // Will provide configuration stuff. For this particular scenario, the configuration
    // is also available throught the RemoteChannelInput_t in the receiver ports.

}UiM_t_contextManager;



void v_UiM_init(RemoteChannelInput_t* pRemoteInputs, UiM_t_Inputs* pInputs);
void v_UiM_update();


#endif;