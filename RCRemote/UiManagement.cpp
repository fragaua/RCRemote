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
Page_t mainPage;


/* Component Declaration */
Component_t_ProgressBar progressBars[N_CHANNELS];
Component_t_Text text_test;

UiC_ErrorType error;


UiM_t_contextManager UiContextManager;
UiM_t_Inputs         UiInputs;

void v_UiM_init(RemoteChannelInput_t* pRemoteInputs, UiM_t_Inputs* pInputs)
{

    UiContextManager.remoteChannelInputs = pRemoteInputs;
    UiContextManager.uiManagementInputs  = pInputs;

    // Initialize UiCore framework. This starts up the display handle and the core context 
    // (later we can maybe chose the handle to use)
    v_UiC_init();
    
    // Initialize all pages
    e_UiC_newPage(&mainPage);

    // Initialize all components
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*5) + (i*2) + 15;
        e_UiC_newProgressBar(&(progressBars[i]), &mainPage, 18, y);
    }

    e_UiC_newText(&text_test, &mainPage, 5, 5);
    v_UiC_updateComponent((Component_t*) &text_test, "Text");
  
}


void v_UiM_update()
{
    // Process input buttons

    // Update all components with received data
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        v_UiC_updateComponent((Component_t*) &(progressBars[i]), &(UiContextManager.remoteChannelInputs[i].u16_Value));
    }

    // Draw current active page
    v_UiC_draw();

}
