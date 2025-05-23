/**
 * @file UiCoreFramework.c
 * @author Marcelo Fraga
 * @brief Contains a set of functionality to enable the abstraction of creating, drawing and updating information
 * for a certain page in the Ui OLED environment of the remote
 * @version 0.1
 * @date 2024 - 06 - 01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "UiCoreFramework.h"


/** Component specific draw and update functions **/
static void drawAnalogMonitorComponent(Component_t_AnalogMonitor* pAnalogMonitor);
static void updateAnalogMonitorComponent(Component_t_AnalogMonitor* pAnalogMonitor, uint16_t* value);

static void drawAnalogAdjustmentComponent(Component_t_AnalogAdjustment* pAnalogAdjust);
static void updateAnalogAdjustmentComponent(Component_t_AnalogAdjustment* pAnalogAdjust, uint32_t* values);

static void drawTextComponent(Component_t_Text* pText);
static void updateTextComponent(Component_t_Text* pText, char* value);

static void drawMenuItemComponent(Component_t_MenuItem* pItem);
static void updateMenuItemComponent(Component_t_MenuItem* pItem, UiC_Input_t* inputs);

static void drawMenuListComponent(Component_t_MenuList* pMenu);
static void updateMenuListComponent(Component_t_MenuList* pMenu, UiC_Input_t* inputs); 

/** Internal UiC functions **/
static void v_UiC_setInternalErrorState(UiC_ErrorType currentError);
static UiC_ErrorType e_UiC_addComponentToPage(Component_t* pComponent, Page_t* pPage);

/// @brief Adds a menu item to the previously created MenuList.
///        Essentially, every MenuItem created needs to have a MenuList created before it, like the following example:
///        Create MenuList -> Create Menu Item 1 -> Create Menu Item 2.
///        In this scenario, Menu Item 1 and Menu Item 2 will be added into MenuList
///        Moreover, the Page where we are adding the MenuItems needs to be the exact same as where we are adding the MenuList, 
///        otherwise and error will be thrown.
///        This isn't the clearest approach but it ensures that we can keep having a generic addComponent without having to pass extra parameters
static UiC_ErrorType e_UiC_addMenuItemToMenu (Component_t_MenuItem* pItem, Page_t* pPage);
static bool b_UiC_isComponentInPage(Component_t* pComponent, Page_t* pPage);


static U8G2_SSD1306 DisplayHandle = U8G2_SSD1306(U8G2_R0, U8X8_PIN_NONE);
static UiCore_t     uiCoreContext;


/**  Core functionality **/

void v_UiC_init()
{

  uiCoreContext.nPages = 0;
  uiCoreContext.currentPage = 0;
  uiCoreContext.internalErrorState = UiC_OK;
  
  // Initialize U8G2 Display Handle
  // TODO: Improve for generic and multiple displays
  DisplayHandle.begin();
  DisplayHandle.clearDisplay();
  DisplayHandle.setFont(u8g2_font_4x6_tf);
  

}

void v_UiC_draw()
{

  DisplayHandle.firstPage();
  do
  {
    uint8_t i;
    Component_t* currentComponent;
    for(i = 0; i < uiCoreContext.currentPage->nComponents; i++)
    {
      currentComponent = uiCoreContext.currentPage->componentList[i]; 
      currentComponent->draw(currentComponent);
    }

  }while(DisplayHandle.nextPage());

}

/** Page Handling  **/

UiC_ErrorType e_UiC_newPage(Page_t* pPage)
{
  if(uiCoreContext.nPages >= MAX_NUMBER_PAGES-1)
  {
    return UiC_ERROR;
  }
  pPage->nComponents = 0;
  uiCoreContext.pageList[uiCoreContext.nPages] = pPage;
  
  // Aditionally, if this is the first created page, assign it to the current page
  if(uiCoreContext.nPages == 0)
  {
    uiCoreContext.currentPage = pPage;
  }

  uiCoreContext.nPages++;
  return UiC_OK;
}

void v_UiC_changePage(Page_t* nextPage)
{
  uiCoreContext.currentPage = nextPage;

}

Page_t* UiC_getActivePage()
{
  return uiCoreContext.currentPage;
}

static UiC_ErrorType e_UiC_addComponentToPage(Component_t* pComponent, Page_t* pPage)
{
  if(pPage->nComponents >= MAX_COMPONENTS_PER_VIEW-1)
  {
    return UiC_ERROR;
  }
  
  // Add the component to the page
  pPage->componentList[pPage->nComponents] = (Component_t*) pComponent;
  pPage->nComponents++; 
  return UiC_OK;
  
}

static UiC_ErrorType e_UiC_addMenuItemToMenu(Component_t_MenuItem* pItem, Page_t* pPage)
{
  /* First we fetch the last added MenuList. We do this by iterating in reverse from the latest items added to the page */
  Component_t_MenuList* pMenu;
  uint8_t i;
  UiC_ErrorType error = UiC_ERROR;
  for(i = (pPage->nComponents - 1); i >= 0; i--)
  {
    if(pPage->componentList[i]->type == UIC_COMPONENT_MENU_LIST)
    {
      error = UiC_OK;
      pMenu = (Component_t_MenuList*) (pPage->componentList[i]);
      break;
    }
  }

  /* Then we can check if the ptr is valid or if we are beyond the nr of items */
  if((pMenu == NULL) || (pMenu->nItems >= MAX_NR_MENU_ITEMS-1))
  {
    return UiC_ERROR;
  }
  
  // Add the component to the page
  pMenu->menuItems[pMenu->nItems] = pItem;
  pMenu->nItems++; 
  
  
  return UiC_OK;
}

static bool b_UiC_isComponentInPage(Component_t* pComponent, Page_t* pPage)
{
  // This isn't very efficient, however, there aren't many components in each page (max ~20) and this is a quick ptr comparison
  // which is equal throughout the program runtime and therefore, I trust that branch predictors will do a good job :)
  uint8_t i;
  for(i = 0; i < pPage->nComponents; i++)
  {
    if(pPage->componentList[i] == pComponent)
    {
      return true;
    }
  }
  return false;

}

/** Error handling  **/
static void v_UiC_setInternalErrorState(UiC_ErrorType currentError)
{
  uiCoreContext.internalErrorState = currentError;
}

UiC_ErrorType UiC_getErrorState()
{
  return uiCoreContext.internalErrorState;
}


/** Component handling **/


UiC_ErrorType e_UiC_addComponent(Component_t* pComponent, Page_t* pPage, ComponentType eComponentType, Component_t_Data componentParameters)
{ 
  //TODO: Create a static init function for each component, to make this function cleaner
  UiC_ErrorType error = UiC_OK;
  // Accessing pos and type directly is safe? Normally, in terms of memory, the "base" from the more complex type we receive here
  // would be in the same place as pos and type from a generic component.

  pComponent->pos.x = componentParameters.x;
  pComponent->pos.y = componentParameters.y;
  pComponent->type  = eComponentType; 

  switch(eComponentType) // TODO: I wanted a LUT for the draw and update functions. I would have to change the types to void*, however this function would be considerably smaller
  {
    case UIC_COMPONENT_TEXT:
      ((Component_t_Text*)pComponent)->base.draw   = (void(*) (Component_t*))        drawTextComponent;
      ((Component_t_Text*)pComponent)->base.update = (void(*) (Component_t*, void*)) updateTextComponent;
      // strncpy <n> parameter takes the size of the <destination>, as per documentation, if <source> is larger, the remainder of the bytes are 0-padded
      // No issues shall arise if we pass in too big of a string.
      strncpy(((Component_t_Text*)pComponent)->value, componentParameters.stringData, sizeof((Component_t_Text*)pComponent)->value); 
    break;

    case UIC_COMPONENT_ANALOGMONITOR:
      ((Component_t_AnalogMonitor*)pComponent)->base.draw   = (void(*) (Component_t*))        drawAnalogMonitorComponent;
      ((Component_t_AnalogMonitor*)pComponent)->base.update = (void(*) (Component_t*, void*)) updateAnalogMonitorComponent;
    break;

    case UIC_COMPONENT_ANALOGADJUSTMENT:
      ((Component_t_AnalogAdjustment*)pComponent)->base.draw   = (void(*) (Component_t*))        drawAnalogAdjustmentComponent;
      ((Component_t_AnalogAdjustment*)pComponent)->base.update = (void(*) (Component_t*, void*)) updateAnalogAdjustmentComponent;
    break;

    case UIC_COMPONENT_MENU_ITEM:
      ((Component_t_MenuItem*)pComponent)->base.draw   = (void(*) (Component_t*))        drawMenuItemComponent;
      ((Component_t_MenuItem*)pComponent)->base.update = (void(*) (Component_t*, void*)) updateMenuItemComponent;
      
      strncpy(((Component_t_MenuItem*)pComponent)->itemText, componentParameters.stringData, sizeof((Component_t_MenuItem*)pComponent)->itemText);

      ((Component_t_MenuItem*)pComponent)->callback = (void(*) (void*))componentParameters.extraData; // Set the callback

      // Aditionally, add the item to the previously created MenuList
      error = e_UiC_addMenuItemToMenu((Component_t_MenuItem*) pComponent, pPage);
    break;

    case UIC_COMPONENT_MENU_LIST:
      ((Component_t_MenuList*)pComponent)->base.draw   = (void(*) (Component_t*))       drawMenuListComponent;
      ((Component_t_MenuList*)pComponent)->base.update = (void(*) (Component_t*, void*))updateMenuListComponent;
    break;
  }

  error = (UiC_ErrorType) (error | e_UiC_addComponentToPage((Component_t*) pComponent, pPage));
  v_UiC_setInternalErrorState(error);

  return error;
}



void v_UiC_updateComponent(Component_t* pComponent, void* pValue)
{
  bool isComponentInPage = b_UiC_isComponentInPage(pComponent, uiCoreContext.currentPage);
  if(isComponentInPage) // Only update if the component is in the current active page
  {
    pComponent->update(pComponent, pValue);
  }
}



/* Component draw and update functions - Display specific functions TODO: implement in another unit? */


// TODO: Implement the various component draw functions. At some point, these can be easily inserted into a "Module Specific" file.
static void drawAnalogMonitorComponent(Component_t_AnalogMonitor* pAnalogMonitor)
{

  DisplayHandle.drawFrame(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, 108, 6);
  DisplayHandle.drawBox(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, map(pAnalogMonitor->value, 0, 1023, 0, 108), 6); // TODO: the map function assumes that we always have values from 0-1023. that's not generic enough for me
}

static void updateAnalogMonitorComponent(Component_t_AnalogMonitor* pAnalogMonitor, uint16_t* value)
{
  pAnalogMonitor->value = *value;
}

static void drawAnalogAdjustmentComponent(Component_t_AnalogAdjustment* pAnalogAdjust)
{
  uint16_t x1 = map(pAnalogAdjust->value1, 0, 1023, 3, 125);// TODO: the map function assumes that we always have values from 0-1023. that's not generic enough for me
  uint16_t x2 = map(pAnalogAdjust->value2, 0, 1023, 3, 125);
  DisplayHandle.drawFrame(pAnalogAdjust->base.pos.x, pAnalogAdjust->base.pos.y, 125, 13);

  DisplayHandle.drawLine(x1, pAnalogAdjust->base.pos.y-3, x1, pAnalogAdjust->base.pos.y+16);
  DisplayHandle.drawLine(x2, pAnalogAdjust->base.pos.y-3, x2, pAnalogAdjust->base.pos.y+16);

  DisplayHandle.setCursor(pAnalogAdjust->value1 > 950 ? x1 - 16 : x1, pAnalogAdjust->base.pos.y + 20); // TODO: Again, too many references to the original analog values. This isn't generic.
  DisplayHandle.print(pAnalogAdjust->value1);

  DisplayHandle.setCursor(pAnalogAdjust->value2 > 950 ? x2 - 16 : x2, pAnalogAdjust->base.pos.y -7);
  DisplayHandle.print(pAnalogAdjust->value2);
  // DisplayHandle.drawLine(x2, 3, x2, 9);
}


static void updateAnalogAdjustmentComponent(Component_t_AnalogAdjustment* pAnalogAdjust, uint32_t* values)
{
  pAnalogAdjust->value1 = (uint16_t)(*values & 0xFFFF);
  pAnalogAdjust->value2 = (uint16_t)(*values >> 16) & 0xFFFF;
}

static void drawTextComponent(Component_t_Text* pText)
{
  DisplayHandle.drawStr(pText->base.pos.x, pText->base.pos.y, pText->value);
}
// TODO: Consider changing char* to const __FlashStringHelper in order to allow for less RAM usage
static void updateTextComponent(Component_t_Text* pText, char* value) 
{
  strncpy(pText->value, value, sizeof(pText->value));
}


// TODO: Improve drawing of menu item and menu
static void drawMenuItemComponent(Component_t_MenuItem* pItem)
{

  DisplayHandle.drawStr(pItem->base.pos.x, pItem->base.pos.y, pItem->itemText);
  if(pItem->isSelected)
  {
    DisplayHandle.drawCircle(pItem->base.pos.x - 3, pItem->base.pos.y - 2, 1);
  }

}

static void updateMenuItemComponent(Component_t_MenuItem* pItem, UiC_Input_t* newState)
{

}

static void drawMenuListComponent(Component_t_MenuList* pMenu)
{
  uint8_t i;

  for(i = 0; i < pMenu->nItems; i++)
  {
    pMenu->menuItems[i]->base.draw((Component_t*) pMenu->menuItems[i]);
  }

}

static void updateMenuListComponent(Component_t_MenuList* pMenu, UiC_Input_t* pInputs)
{
  uint8_t i;
    
  // 'De-select' all menu entries and ultimately select the one calculated below.
  // Also restart the selectedMenuItem and remove any clicked item
  pMenu->menuItems[pMenu->currentlySelectedIdx]->isClicked = false;
  for(i = 0; i < pMenu->nItems; i++)
  {
    pMenu->menuItems[i]->isSelected = false;
  }

  if(pInputs->inputDown)
  {
    pMenu->currentlySelectedIdx = (pMenu->currentlySelectedIdx + 1) % pMenu->nItems; 
  }
  else if(pInputs->inputUp)
  {
    pMenu->currentlySelectedIdx = (pMenu->currentlySelectedIdx == 0) ? pMenu->nItems-1 : pMenu->currentlySelectedIdx -1; 
  }
  else if(pInputs->inputSelect)
  {
    pMenu->menuItems[pMenu->currentlySelectedIdx]->isClicked = true; // Set the internal "clicked" state of button
    
    if(pMenu->menuItems[pMenu->currentlySelectedIdx]->callback != NULL)
    {
      pMenu->menuItems[pMenu->currentlySelectedIdx]->callback((void*) pMenu->currentlySelectedIdx);   // Call the "callback" function to execute an action
    }

  }

  pMenu->menuItems[pMenu->currentlySelectedIdx]->isSelected = true;
}
