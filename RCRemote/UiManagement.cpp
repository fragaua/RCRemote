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
Page_t configurationPage; // Where we configure the current channel


/* Component Declaration */

#define N_OPTIONS 3u

Component_t_AnalogMonitor progressBars[N_CHANNELS];
Component_t_AnalogAdjustment adjustmentBar;
Component_t_MenuItem    analogId[N_CHANNELS];
Component_t_MenuItem    communicationState;

// DEBUG
Component_t_Text        testButton1;
Component_t_Text        testButton2;

Component_t_MenuList    optionsMenu;
Component_t_MenuItem    options[N_OPTIONS];

Component_t_MenuList    channelChooseMenu;


Component_t_Data componentInputData;


UiC_ErrorType error;


static UiM_t_contextManager UiContextManager;


/** Internal function declaration **/
static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs);
static bool b_UiM_risingEdge(uint8_t prevValue, uint8_t currentValue, uint8_t desiredEdge);
static bool b_UiM_buttonHold(bool risingEdge, bool currentValue, unsigned long* timeHolding);
static void v_UiM_updateProviderPorts(void);


/**  Project Specific functions  **/ // Todo: eventually we can have a separate project specific file.
static void buildCommunicationString(bool connectionDropped, unsigned long txTime, char* commStateString);
static void switchToConfigurationOptionsPage(void* selectedChannelIdx);
static void switchToConfigurationPage(void* selectedConfigurationIdx);
static void updateAdjustmentMonitors(uint16_t* adjustmentWheel, uint16_t updateNextValueButton);


void v_UiM_init(UiM_t_rPorts* pReceiverPorts, UiM_t_pPorts* pProviderPorts)
{

    uint8_t i;
    
    UiContextManager.rPorts = pReceiverPorts;
    UiContextManager.pPorts = pProviderPorts;

    // Initialize UiCore framework. This starts up the display handle and the core context 
    // (later we can maybe chose the handle to use)
    v_UiC_init();
    
    // Initialize all pages
    e_UiC_newPage(&monitoringPage);
    e_UiC_newPage(&optionsPage);
    e_UiC_newPage(&configurationPage);


    // Initialize all components
    e_UiC_addComponent((Component_t*)&channelChooseMenu,      &monitoringPage, UIC_COMPONENT_MENU_LIST, {0});
    
    for(i = 0; i < N_CHANNELS; i++)
    {
        uint8_t y = (i*7) + 15; // TODO: Improve this
        e_UiC_addComponent((Component_t*)&(analogId[i]),       &monitoringPage, UIC_COMPONENT_MENU_ITEM,  {3,  y+5, pReceiverPorts->remoteChannelInputs[i].c_Name, (void*) switchToConfigurationOptionsPage}); // TODO call back needs to be caleld with the parameter to which channel to update.
        e_UiC_addComponent((Component_t*)&(progressBars[i]),   &monitoringPage, UIC_COMPONENT_ANALOGMONITOR, {18, y, NULL});

    }
    
    e_UiC_addComponent((Component_t*)&optionsMenu,             &optionsPage,    UIC_COMPONENT_MENU_LIST, {0});
    e_UiC_addComponent((Component_t*)&(options[0]),            &optionsPage,    UIC_COMPONENT_MENU_ITEM, {3, 20, "Trimming", (void*) switchToConfigurationPage});
    e_UiC_addComponent((Component_t*)&(options[1]),            &optionsPage,    UIC_COMPONENT_MENU_ITEM, {3, 27, "EndPont",  (void*) switchToConfigurationPage});
    e_UiC_addComponent((Component_t*)&(options[2]),            &optionsPage,    UIC_COMPONENT_MENU_ITEM, {3, 34, "Invert",   (void*) switchToConfigurationPage});

    

    // DEBUG    
    e_UiC_addComponent((Component_t*) &(testButton1),          &optionsPage,     UIC_COMPONENT_TEXT, {35, 5, ""});
    e_UiC_addComponent((Component_t*) &(testButton2),          &monitoringPage,  UIC_COMPONENT_TEXT, {35, 5, ""});
    e_UiC_addComponent((Component_t*) &(communicationState),   &monitoringPage,  UIC_COMPONENT_TEXT,  {1, 5, "NoComm"});
    
    e_UiC_addComponent((Component_t*) &(adjustmentBar),        &configurationPage,  UIC_COMPONENT_ANALOGADJUSTMENT,  {2, 30});


    Serial.println(UiC_getErrorState());

}


void v_UiM_update()
{
    char commStateStr[MAX_NR_CHARS] = "NCom";
    char tb1[7];
    uint8_t i;

    // DEBUG
    snprintf(tb1, 7, "%d %d %d", UiContextManager.rPorts->uiManagementInputs->inputButtonLeft, 
                                 UiContextManager.rPorts->uiManagementInputs->inputButtonSelect, 
                                 UiContextManager.rPorts->uiManagementInputs->inputButtonRight);
    buildCommunicationString(UiContextManager.rPorts->remoteCommState->b_ConnectionLost, UiContextManager.rPorts->remoteCommState->l_TransmissionTime, commStateStr);
    
    // Process input buttons
    v_UiM_processUIManagementInputs(UiContextManager.rPorts->uiManagementInputs);

    // Update pages (Temporary: for now, on every page, if I hold the Left button it goes back to monitoring
    if(UiContextManager.rPorts->uiManagementInputs->holdButtonLeft)
    {
        v_UiC_changePage(&monitoringPage);
    }

    
    // Update all components with received data
    for(i = 0; i < N_CHANNELS; i++)
    {
        v_UiC_updateComponent((Component_t*) &(progressBars[i]), &(UiContextManager.rPorts->remoteChannelInputs[i].u16_Value));
    }

    // TODO: remove this temp
    UiC_Input_t temp = {UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonLeft, 
                        UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonRight, 
                        UiContextManager.rPorts->uiManagementInputs->risingEdgeButtonSelect}; 
    v_UiC_updateComponent((Component_t*) &optionsMenu, (void*) &temp);
    v_UiC_updateComponent((Component_t*) &channelChooseMenu, (void*) &temp);
    v_UiC_updateComponent((Component_t*) &(communicationState), (void*) commStateStr);
    v_UiC_updateComponent((Component_t*) &(testButton1),        (void*) tb1);
    v_UiC_updateComponent((Component_t*) &(testButton2),        (void*) tb1);
    
    updateAdjustmentMonitors(&(UiContextManager.rPorts->uiManagementInputs->scrollWheel), UiContextManager.rPorts->uiManagementInputs->holdButtonSelect);



    v_UiM_updateProviderPorts();

    // Draw current active page
    v_UiC_draw();

}


static void v_UiM_updateProviderPorts(void)
{
    // For now, updates the "analogSendAllowed" based on the current page.
    UiContextManager.pPorts->analogSendAllowed = false;
    Page_t* activePage = UiC_getActivePage();

    if(activePage == &monitoringPage)
    {
        // The analog send is only allowed on the monitoring page.
        UiContextManager.pPorts->analogSendAllowed = true;
    }
    else if(activePage == &configurationPage)
    {
        // If we leave this page by clicking "go" or smthng, save the configuration (in this case it's provided through the rports)
    }
}

static void v_UiM_processUIManagementInputs(UiM_t_Inputs* uiInputs)
{  
    static unsigned long timeHoldingSelect = 0;
    static unsigned long timeHoldingLeft   = 0;
    static unsigned long timeHoldingRight  = 0;

    uiInputs->risingEdgeButtonSelect       = b_UiM_risingEdge(uiInputs->prevButtonSelect,   uiInputs->inputButtonSelect, HIGH);
    uiInputs->risingEdgeButtonLeft         = b_UiM_risingEdge(uiInputs->prevButtonLeft,     uiInputs->inputButtonLeft,   HIGH);
    uiInputs->risingEdgeButtonRight        = b_UiM_risingEdge(uiInputs->prevButtonRight,    uiInputs->inputButtonRight,  HIGH);
    
    uiInputs->holdButtonSelect             = b_UiM_buttonHold(uiInputs->risingEdgeButtonSelect, uiInputs->inputButtonSelect, &timeHoldingSelect);
    uiInputs->holdButtonLeft               = b_UiM_buttonHold(uiInputs->risingEdgeButtonLeft,   uiInputs->inputButtonLeft,   &timeHoldingLeft);
    uiInputs->holdButtonRight              = b_UiM_buttonHold(uiInputs->risingEdgeButtonRight,  uiInputs->inputButtonRight,  &timeHoldingRight);

    // Update button states for next cycle
    uiInputs->prevButtonSelect             = uiInputs->inputButtonSelect;
    uiInputs->prevButtonLeft               = uiInputs->inputButtonLeft;
    uiInputs->prevButtonRight              = uiInputs->inputButtonRight;

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


static void switchToConfigurationOptionsPage(void* selectedChannelIdx)
{
    v_UiC_changePage(&optionsPage);
}

static void switchToConfigurationPage(void* selectedConfigurationIdx)
{
    v_UiC_changePage(&configurationPage);
    // TODO:
    // Set the configuration channel text to the one we selected
    // Set the title to the configuration option we selected
}


static void updateAdjustmentMonitors(uint16_t* adjustmentWheel, uint16_t updateNextValueButton)
{
    uint32_t updateValue;
    static uint16_t lastValueBeforeUpdating;
    static bool updateNextValue = false;
    if(UiC_getActivePage() == &configurationPage)
    {
        if(!updateNextValue && updateNextValueButton) // If select button was hit and we are currently on the first value, toggle the update variable
        {
            updateNextValue = !updateNextValue;
            lastValueBeforeUpdating = (*adjustmentWheel); // Make sure to save the configurated value before configuring the next one.
        }

        updateValue = updateNextValue ? ((((uint32_t)*adjustmentWheel) << 16) | ((uint32_t)lastValueBeforeUpdating & 0xFFFF)) : (uint32_t)(*adjustmentWheel); // TODO: complex expression, wrap some macros for bit management here
        v_UiC_updateComponent((Component_t*) &(adjustmentBar), (void*) (&updateValue)); // TODO: dont use globals here
    }
    else
    {
        updateNextValue = false;
    }

}
