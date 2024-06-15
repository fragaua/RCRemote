/**
 * @file UiManagement.h 
 * @author Marcelo Fraga
 * @brief Source file for UiManagement. This is a project specific component where the UiCore
 * is initialized and all pages and components are instantiated and managed. Inputs are passed
 * down from the original application (or any other module for that matter) in order to make changes
 * to the current UiCore Context manager.
 * @version 0.1
 * @date 2024 - 06 - 14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "UiManagement.h"

/* Page/View declaration */
Page_t monitoringPage;  // Default page where we can see the the analog monitors etc.
Page_t optionsMenu;     // Options menu. Contains a group of options and allows us to navigate to other pages such as configuration
Page_t configurationPage; // Where we would configure trimming, channel inversion or endpoint adjustment.


/* Component Declaration */
Component_t_ProgressBar progressBars[N_CHANNELS];
Component_t_Text        analogId[N_CHANNELS];
Component_t_Text        communicationState;
Component_t_Text        test;

UiC_ErrorType error;


static UiM_t_contextManager UiContextManager;


/** Internal function declaration **/
static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs);
static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge);

void v_UiM_init(UiM_t_rPorts* pReceiverPorts)
{

    UiContextManager.rPorts = pReceiverPorts;
    // Initialize UiCore framework. This starts up the display handle and the core context 
    // (later we can maybe chose the handle to use)
    v_UiC_init();
    
    // Initialize all pages
    e_UiC_newPage(&monitoringPage);
    e_UiC_newPage(&optionsMenu);

    // Initialize all components
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*5) + (i*2) + 15;
        e_UiC_newText(&(analogId[i]), &monitoringPage, 1, y+5);
        v_UiC_updateComponent((Component_t*) &(analogId[i]), (void*) pReceiverPorts->remoteChannelInputs[i].c_Name);
        e_UiC_newProgressBar(&(progressBars[i]), &monitoringPage, 18, y);
    }
    e_UiC_newText(&(communicationState), &monitoringPage, 1, 5);  
    
    e_UiC_newText(&(test), &optionsMenu, 1, 5);  
    v_UiC_updateComponent((Component_t*) &(test), (void*) "Options Menu");

}


void v_UiM_update()
{

    // Process input buttons
    v_UiM_processUIManagementInputs(UiContextManager.rPorts->uiManagementInputs);
    
    // Update all components with received data // TODO: Only components in current page should be updated.
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        v_UiC_updateComponent((Component_t*) &(progressBars[i]), &(UiContextManager.rPorts->remoteChannelInputs[i].u16_Value));

    }
    
    // TODO: Put into function
    char commStateStr[MAX_NR_CHARS] = "NoComm";
    if(!UiContextManager.rPorts->remoteCommState->b_ConnectionLost)
    {
        snprintf(commStateStr, MAX_NR_CHARS, "%d US", UiContextManager.rPorts->remoteCommState->l_TransmissionTime);
    }
    v_UiC_updateComponent((Component_t*) &(communicationState), (void*) commStateStr);

    // Draw current active page
    v_UiC_draw();

}



static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs)
{
    static uint8_t prevValueButtonSelect;
    static uint8_t x = 0; // Temporary to go back and forth the pages. This should be fetched from the UiC 
    // For now, lets take the select button and whenever it's clicked, change the displayed page.
    // Serial.print("Button left: ");
    // Serial.println(uiInputs->inputButtonLeft);
    bool risingEdge = b_UiM_risingEdge(prevValueButtonSelect, uiInputs->inputButtonSelect, HIGH);

    if(risingEdge)
    {
        if(x == 0)
        {
            x = 1;
            v_UiC_changePage(&optionsMenu);
        }
        else
        {
            x = 0;
            v_UiC_changePage(&monitoringPage);
        }
    }

    prevValueButtonSelect = uiInputs->inputButtonSelect;

}


static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge)
{
    return (prevValue != currentValue) && (currentValue == desiredEdge);
}

