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


UiC_ErrorType error;


static UiM_t_contextManager UiContextManager;

void v_UiM_init(RemoteChannelInput_t* pRemoteInputs, UiM_t_Inputs* pInputs)
{

    UiContextManager.remoteChannelInputs = pRemoteInputs;
    UiContextManager.uiManagementInputs  = pInputs;

    // Initialize UiCore framework. This starts up the display handle and the core context 
    // (later we can maybe chose the handle to use)
    v_UiC_init();
    
    // Initialize all pages
    e_UiC_newPage(&monitoringPage);

    // Initialize all components
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*5) + (i*2) + 15;
        e_UiC_newText(&(analogId[i]), &monitoringPage, 1, y+5);
        v_UiC_updateComponent((Component_t*) &(analogId[i]), (void*) pRemoteInputs[i].c_Name);
        e_UiC_newProgressBar(&(progressBars[i]), &monitoringPage, 18, y);

    }
  
}


void v_UiM_update()
{
    // Process input buttons
    // Update all components with received data // TODO: Only components in current page should be updated.
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        v_UiC_updateComponent((Component_t*) &(progressBars[i]), &(UiContextManager.remoteChannelInputs[i].u16_Value));
    }

    // Draw current active page
    v_UiC_draw();

}
