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
Page_t optionsPage;     // Options menu. Contains a group of options and allows us to navigate to other pages such as configuration
Page_t configurationPage; // Where we would configure trimming, channel inversion or endpoint adjustment.


/* Component Declaration */
Component_t_ProgressBar progressBars[N_CHANNELS];
Component_t_Text        analogId[N_CHANNELS];
Component_t_Text        communicationState;
Component_t_Text        test;

Component_t_MenuList    optionsMenu;
Component_t_MenuItem    options[3];

Component_t_Data componentInputData;


UiC_ErrorType error;


static UiM_t_contextManager UiContextManager;


/** Internal function declaration **/
static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs);
static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge);
static bool b_UiM_buttonHold(bool risingEdge, bool currentValue, unsigned long* timeHolding);

void v_UiM_init(UiM_t_rPorts* pReceiverPorts)
{

    UiContextManager.rPorts = pReceiverPorts;
    // Initialize UiCore framework. This starts up the display handle and the core context 
    // (later we can maybe chose the handle to use)
    v_UiC_init();
    
    // Initialize all pages
    e_UiC_newPage(&monitoringPage);
    e_UiC_newPage(&optionsPage);

    // Initialize all components
    error = e_UiC_addComponent((Component_t*)&optionsMenu, &optionsPage, UIC_COMPONENT_MENU_LIST, {0});

    uint8_t i;
    for(i = 0; i < 3; i++)
    {
        uint8_t y = (i*7) + 15;
        error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(options[i]), &optionsPage, UIC_COMPONENT_MENU_ITEM, {3, y, "Item"}));
    }
    
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*7) + 20;
        error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(analogId[i]),     &monitoringPage, UIC_COMPONENT_TEXT,        {1,  y, pReceiverPorts->remoteChannelInputs[i].c_Name}));
        error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(progressBars[i]), &monitoringPage, UIC_COMPONENT_PROGRESSBAR, {18, y, NULL}));
    }
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*) &(communicationState), &monitoringPage, UIC_COMPONENT_TEXT, {1, 5, "NoComm"}));
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*) &(test),               &optionsPage,    UIC_COMPONENT_TEXT, {1, 5, "Options"}));

    Serial.println(error);
}


void v_UiM_update()
{

    // Process input buttons
    v_UiM_processUIManagementInputs(UiContextManager.rPorts->uiManagementInputs);

    // Update pages // TODO: put into function
    if(UiContextManager.rPorts->uiManagementInputs->holdButtonSelect)
    {
        v_UiC_changePage(&optionsPage);
    }
    else if(UiContextManager.rPorts->uiManagementInputs->holdButtonRight)
    {
        v_UiC_changePage(&monitoringPage);
    }
    
    // Update all components with received data // TODO: Put into function
    v_UiC_updateComponent((Component_t*) &optionsMenu, (void*) UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonLeft);

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


// TODO: Separate this function into "process page change ui input" and others like "process current page ui input"
// TODO: Make this function return the 'rising edges' or something, to give as input to the update processes
static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs)
{  
    static unsigned long timeHoldingSelect = 0;
    static unsigned long timeHoldingLeft   = 0;
    static unsigned long timeHoldingRight  = 0;

    uiInputs->risingEdgeButtonSelect = b_UiM_risingEdge(uiInputs->prevButtonSelect,   uiInputs->inputButtonSelect, HIGH);
    uiInputs->risingEdgeButtonLeft   = b_UiM_risingEdge(uiInputs->prevButtonLeft,     uiInputs->inputButtonLeft,   HIGH);
    uiInputs->risingEdgeButtonRight  = b_UiM_risingEdge(uiInputs->prevButtonRight,    uiInputs->inputButtonRight,  HIGH);
    
    uiInputs->holdButtonSelect       = b_UiM_buttonHold(uiInputs->risingEdgeButtonSelect, uiInputs->inputButtonSelect, &timeHoldingSelect);
    uiInputs->holdButtonLeft         = b_UiM_buttonHold(uiInputs->risingEdgeButtonLeft,   uiInputs->inputButtonLeft,   &timeHoldingLeft);
    uiInputs->holdButtonRight        = b_UiM_buttonHold(uiInputs->risingEdgeButtonRight,  uiInputs->inputButtonRight,  &timeHoldingRight);

    // Update button states for next cycle
    uiInputs->prevButtonSelect   = uiInputs->inputButtonSelect;
    uiInputs->prevButtonLeft     = uiInputs->inputButtonLeft;
    uiInputs->prevButtonRight    = uiInputs->inputButtonRight;

}


static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge)
{
    return (prevValue != currentValue) && (currentValue == desiredEdge);
}

static bool b_UiM_buttonHold(bool risingEdge, bool currentValue, unsigned long* timeHolding)
{
    if(risingEdge)
    {
        *timeHolding = millis();
    }

    return ((millis() - *timeHolding) > 1500) && (currentValue == HIGH);
}
