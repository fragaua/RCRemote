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
static void drawProgressBarComponent(Component_t_ProgressBar* pProgressBar);
static void updateProgressBarComponent(Component_t_ProgressBar* pProgressBar, uint16_t* value);

static void drawTextComponent(Component_t_Text* pText);
static void updateTextComponent(Component_t_Text* pText, char* value);

static void drawMenuItemComponent(Component_t_MenuItem* pItem);
static void updateMenuItemComponent(Component_t_MenuItem* pItem, bool* newState); // TODO: see what more info we need to update button

static void drawMenuListComponent(Component_t_MenuList* pMenu);
static void updateMenuListComponent(Component_t_MenuList* pMenu, bool* nextItem); // TODO: see what more info we need to update list

/** Internal UiC functions **/
static UiC_ErrorType e_UiC_addComponentToPage(Component_t* pComponent, Page_t* pPage);
// static UiC_ErrorType e_UiC_addMenuItemToMenu (Component_t_MenuItem* pItem, Component_t_MenuList* pMenu); // Temporary to solve an issue


static U8G2_SSD1306 DisplayHandle = U8G2_SSD1306(U8G2_R0, U8X8_PIN_NONE);
static UiCore_t     uiCoreContext;


/**  Core functionality **/

void v_UiC_init()
{

  uiCoreContext.nPages = 0;
  uiCoreContext.currentPage = 0;
  
  // Initialize U8G2 Display Handle
  DisplayHandle.begin();
  DisplayHandle.clearDisplay();
  DisplayHandle.setFont(u8g2_font_smolfont_tf);
  

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

UiC_ErrorType e_UiC_addMenuItemToMenu(Component_t_MenuItem* pItem, Component_t_MenuList* pMenu)
{
  if(pMenu->nItems >= MAX_NR_MENU_ITEMS-1)
  {
    return UiC_ERROR;
  }
  
  // Add the component to the page
  pMenu->menuItems[pMenu->nItems] = pItem;
  pMenu->nItems++; 
  return UiC_OK;
}

/** Component handling **/


UiC_ErrorType e_UiC_addComponent(Component_t* pComponent, Page_t* pPage, ComponentType eComponentType, Component_t_Data baseData)
{
  // Accessing pos and type directly is safe? Normally, in terms of memory, the "base" from the more complex type we receive here
  // would be in the same place as pos and type from a generic component.
  pComponent->pos.x = baseData.x;
  pComponent->pos.y = baseData.y;
  pComponent->type = eComponentType; 
  switch(eComponentType) // TODO: I wanted a LUT for the draw and update functions. I would have to change the types to void*, however this function would be considerably smaller
  {
    case UIC_COMPONENT_TEXT:
      ((Component_t_Text*)pComponent)->base.draw = (void(*) (Component_t*))drawTextComponent;
      ((Component_t_Text*)pComponent)->base.update = (void(*)(Component_t*, void*))updateTextComponent;
      // strncpy <n> parameter takes the size of the <destination>, as per documentation, if <source> is larger, the remainder of the bytes are 0-padded
      // No issues shall arise if we pass in too big of a string.
      strncpy(((Component_t_Text*)pComponent)->value, baseData.stringData, sizeof((Component_t_Text*)pComponent)->value); 
    break;

    case UIC_COMPONENT_PROGRESSBAR:
      ((Component_t_ProgressBar*)pComponent)->base.draw = (void(*) (Component_t*))drawProgressBarComponent;
      ((Component_t_ProgressBar*)pComponent)->base.update = (void(*)(Component_t*, void*))updateProgressBarComponent;
    break;

    case UIC_COMPONENT_MENU_ITEM:
      ((Component_t_MenuItem*)pComponent)->base.draw = (void(*) (Component_t*))drawMenuItemComponent;
      ((Component_t_MenuItem*)pComponent)->base.update = (void(*)(Component_t*, void*))updateMenuItemComponent;
      strncpy(((Component_t_MenuItem*)pComponent)->itemText, baseData.stringData, sizeof((Component_t_MenuItem*)pComponent)->itemText);
    break;

    case UIC_COMPONENT_MENU_LIST:
      ((Component_t_MenuList*)pComponent)->base.draw = (void(*) (Component_t*))drawMenuListComponent;
      ((Component_t_MenuList*)pComponent)->base.update = (void(*)(Component_t*, void*))updateMenuListComponent;
    break;
  }
  return e_UiC_addComponentToPage((Component_t*) pComponent, pPage);
}



void v_UiC_updateComponent(Component_t* pComponent, void* pValue)
{
  pComponent->update(pComponent, pValue);
}



/* Component draw and update functions - Display specific functions TODO: implement in another unit? */


// TODO: Implement the various component draw functions. At some point, these can be easily inserted into a "Module Specific" file.
static void drawProgressBarComponent(Component_t_ProgressBar* pProgressBar)
{

  DisplayHandle.drawFrame(pProgressBar->base.pos.x, pProgressBar->base.pos.y, 108, 6);
  DisplayHandle.drawBox(pProgressBar->base.pos.x, pProgressBar->base.pos.y, map(pProgressBar->value, 0, 1023, 108, 0), 6);
}

static void updateProgressBarComponent(Component_t_ProgressBar* pProgressBar, uint16_t* value)
{
  pProgressBar->value = *value;
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
    DisplayHandle.drawCircle(pItem->base.pos.x - 2, pItem->base.pos.y - 2, 1);
  }

}

static void updateMenuItemComponent(Component_t_MenuItem* pItem, bool* newState)
{
  pItem->isSelected = *newState;
}

static void drawMenuListComponent(Component_t_MenuList* pMenu)
{
  uint8_t i;

  for(i = 0; i < pMenu->nItems; i++)
  {
    pMenu->menuItems[i]->base.draw((Component_t*) pMenu->menuItems[i]);
  }

}

// TODO: Eventually we need to also include here the input for "select button" if a draw update needs to take place.
static void updateMenuListComponent(Component_t_MenuList* pMenu, bool* nextItem)
{
  uint8_t i;
  if(*nextItem) // nextItem is a 'one-cycle' sort of signal meaning it should be active once every time we want to shift to the next menu entry
  {
    pMenu->currentlySelectedIdx = (pMenu->currentlySelectedIdx + 1) % pMenu->nItems;
  }

  // 'De-select' all menu entries and ultimately select the one calculated above.
  for(i = 0; i < pMenu->nItems; i++)
  {
    pMenu->menuItems[i]->isSelected = false;
  }
  Serial.println(pMenu->currentlySelectedIdx);
  pMenu->menuItems[pMenu->currentlySelectedIdx]->isSelected = true;
}
