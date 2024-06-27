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

// DEBUG
Component_t_Text        testButton1;
Component_t_Text        testButton2;

Component_t_MenuList    optionsMenu;
Component_t_MenuItem    options[3];

Component_t_Data componentInputData;


UiC_ErrorType error;


static UiM_t_contextManager UiContextManager;


/** Internal function declaration **/
static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs);
static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge);
static bool b_UiM_buttonHold(bool risingEdge, bool currentValue, unsigned long* timeHolding);


/**  Project Specific functions  **/ // Todo: eventually we can have a separate project specific file.
static void buildCommunicationString(bool connectionDropped, unsigned long txTime, char* commStateString);
static void switchTrimmingPage(void);


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
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(options[0]), &optionsPage, UIC_COMPONENT_MENU_ITEM, {3, 20,  "Trimming", (void*) switchTrimmingPage}));
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(options[1]), &optionsPage, UIC_COMPONENT_MENU_ITEM, {3, 27, "EndPointAdj"}));
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(options[2]), &optionsPage, UIC_COMPONENT_MENU_ITEM, {3, 34, "Invert"}));
    
    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*7) + 20;
        uint8_t y2 = (i*7) + 15; // TODO: Improve this
        error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(analogId[i]),     &monitoringPage, UIC_COMPONENT_TEXT,        {1,  y, pReceiverPorts->remoteChannelInputs[i].c_Name}));
        error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*)&(progressBars[i]), &monitoringPage, UIC_COMPONENT_PROGRESSBAR, {18, y2, NULL}));
    }
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*) &(communicationState), &monitoringPage, UIC_COMPONENT_TEXT, {1, 5, "NoComm"}));

    // DEBUG    
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*) &(testButton1),               &optionsPage,    UIC_COMPONENT_TEXT, {35, 5, ""}));
    error = (UiC_ErrorType)(error | e_UiC_addComponent((Component_t*) &(testButton2),               &monitoringPage,    UIC_COMPONENT_TEXT, {35, 5, ""}));


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
    
    // Update all components with received data // TODO: Put into function and remove this temp
    UiC_Input_t temp = {UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonLeft, 
                        UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonRight, 
                        UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonSelect};

    v_UiC_updateComponent((Component_t*) &optionsMenu, (void*) &temp);
    

    uint8_t i;
    for(i = 0; i < N_CHANNELS; i++)
    {
        v_UiC_updateComponent((Component_t*) &(progressBars[i]), &(UiContextManager.rPorts->remoteChannelInputs[i].u16_Value));
    }

    char commStateStr[MAX_NR_CHARS] = "NoComm";
    buildCommunicationString(UiContextManager.rPorts->remoteCommState->b_ConnectionLost, UiContextManager.rPorts->remoteCommState->l_TransmissionTime, commStateStr);
    v_UiC_updateComponent((Component_t*) &(communicationState), (void*) commStateStr);

    

    // DEBUG
    char tb1[7];
    snprintf(tb1, 7, "%d %d %d", UiContextManager.rPorts->uiManagementInputs->inputButtonLeft, UiContextManager.rPorts->uiManagementInputs->inputButtonSelect, UiContextManager.rPorts->uiManagementInputs->inputButtonRight);

    v_UiC_updateComponent((Component_t*) &(testButton1), (void*) tb1);
    v_UiC_updateComponent((Component_t*) &(testButton2), (void*) tb1);



    // Draw current active page
    v_UiC_draw();

}


static void v_UiM_pageManagement(UiM_t_Inputs* uiInputs, Page_t* currentPage)
{

    
}

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



/** Project specific **/
static void buildCommunicationString(bool connectionDropped, unsigned long txTime, char* commStateString)
{
    if(!connectionDropped)
    {
        snprintf(commStateString, MAX_NR_CHARS, "%d US", txTime);
    }
}


static void switchTrimmingPage(void)
{
    v_UiC_changePage(&monitoringPage);
}