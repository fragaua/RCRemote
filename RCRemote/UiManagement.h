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



// Contains all the inputs the application is able to handle for User Interaction as well as processed versions (rising edge, hold..)
typedef struct UiM_t_Inputs
{
    // Project specific set of Ui interface
    uint16_t     inputButtonLeft        : 1;
    uint16_t     inputButtonRight       : 1;
    uint16_t     inputButtonSelect      : 1;

    uint16_t     risingEdgeButtonLeft   : 1;
    uint16_t     risingEdgeButtonRight  : 1;
    uint16_t     risingEdgeButtonSelect : 1;

    uint16_t     holdButtonLeft         : 1;
    uint16_t     holdButtonRight        : 1;
    uint16_t     holdButtonSelect       : 1;

    uint16_t     prevButtonLeft         : 1;
    uint16_t     prevButtonRight        : 1;
    uint16_t     prevButtonSelect       : 1;

    uint16_t scrollWheel; 

}UiM_t_Inputs;

// Contains all necessary data to update and manage the ui - To be updated with project specific information
typedef struct UiM_t_rPorts
{

    UiM_t_Inputs*               uiManagementInputs;
    RemoteChannelInput_t*       remoteChannelInputs;
    RemoteCommunicationState_t* remoteCommState;

}UiM_t_rPorts;

// Contains all necessary data to inform the UiManagement caller of changes. This is project specific information
typedef struct UiM_t_pPorts
{
    bool analogSendAllowed : 1;

    // For this particular project we only need a 'configuration updated' flag, since the configuration pointer is passed through the rports
    // This is activated once after a config update and then replaced with 0 again.
    bool configurationUpdated : 1; 

}UiM_t_pPorts;


// Global variables that need sharing across project specific functions. This is project specific information.
typedef struct UiM_t_Globals
{   

    uint8_t channelMenuSelectedOptionIdx       : 3;
    uint8_t configurationMenuSelectedOptionIdx : 3;

}UiM_t_Globals;


// Contains all the inputs necessary to update the internal components.
typedef struct UiM_t_contextManager
{
    /**** Receiver Ports *****/
    // Project specific data to display and inputs to manage ui
    UiM_t_rPorts* rPorts;


    /**** Provider Ports *****/
    // Will provide configuration stuff. For this particular scenario, the configuration
    // is also available through the RemoteChannelInput_t in the receiver ports.
    UiM_t_pPorts* pPorts;


    /**** Globals ******/
    // Management variables used to control the flow in a project specific UI.
    // For instance, could be used to save the output state of a callback for a selected button
    // in order to use later
    UiM_t_Globals globals;

}UiM_t_contextManager;


void v_UiM_init(UiM_t_rPorts* pReceiverPorts, UiM_t_pPorts* pProviderPorts);
void v_UiM_update();


#endif;