/**
 * @file UiManagement.h 
 * @author Marcelo Fraga
 * @brief Header file for UiManagement. This is a project specific component where the UiCore
 * is initialized and all pages and components are instantiated and managed. Inputs are passed
 * down from the original application (or any other module for that matter) in order to make changes
 * to the current UiCore Context manager.
 * 
 * While this particular unit isn't generic, it should be easy to adapt to any project specific scenario.
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
    // Project specific data to display
    RemoteChannelInput_t* remoteChannelInputs; 
    
    // Project specific Ui management input data to manage
    UiM_t_Inputs*         uiManagementInputs;

}UiM_t_contextManager;



void v_UiM_init(RemoteChannelInput_t* pRemoteInputs, UiM_t_Inputs* pInputs);
void v_UiM_update();

UiM_t_Inputs* UiM_getInputsPointer(void);


#endif;