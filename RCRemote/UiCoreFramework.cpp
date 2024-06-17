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
static void updateMenuListComponent(Component_t_MenuList* pMenu, bool* newState); // TODO: see what more info we need to update list

/** Internal UiC functions **/
static UiC_ErrorType e_UiC_addComponentToPage(Component_t* pComponent, Page_t* pPage);
static UiC_ErrorType e_UiC_addMenuItemToMenu (Component_t_MenuItem* pItem, Component_t_MenuList* pMenu);


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
    for(i = 0; i < uiCoreContext.pageList[uiCoreContext.currentPage]->nComponents; i++)
    {
      currentComponent = uiCoreContext.pageList[uiCoreContext.currentPage]->componentList[i]; 
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
  uiCoreContext.nPages++;
  return UiC_OK;
}

// TODO: Provide the ability to change to the next page on the list 
void v_UiC_changePage(Page_t* nextPage)
{
  uint8_t i; 
  for(i = 0; i < MAX_NUMBER_PAGES; i++) // For now, let's iterate the pages to fetch the index of "nextPage". TODO: Improve this
  {
    if(nextPage == uiCoreContext.pageList[i])
    {
      break;
    }
  }
  // TODO: Make sure we have a valid page and throw error if we don't

  // Update the current page to the index of "nextPage"
  uiCoreContext.currentPage = i;

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

static UiC_ErrorType e_UiC_addMenuItemToMenu(Component_t_MenuItem* pItem, Component_t_MenuList* pMenu)
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

// TODO: Can we make a generic component add function? Draw and update functions could be fetched via
// LUT (like it was at a certain point) and the value could be responsability of the updateCOmponent function.
// Position values could easily be generic since every component already has it anyway
UiC_ErrorType e_UiC_newText(Component_t_Text* pText, Page_t* pPage, uint8_t x, uint8_t y)
{
  // Initialize component
  // Assign function and cast child component to parent Component_t
  pText->base.draw   = (void(*)(Component_t*))drawTextComponent; 
  pText->base.update = (void(*)(Component_t*, void*))updateTextComponent; 
  pText->base.type   = UIC_COMPONENT_TEXT;
  pText->base.pos.x  = x;
  pText->base.pos.y  = y;
  
  return e_UiC_addComponentToPage((Component_t*) pText, pPage);

}

UiC_ErrorType e_UiC_newProgressBar(Component_t_ProgressBar* pProgressBar, Page_t* pPage, uint8_t x, uint8_t y)
{  
  // Initialize component
  // Assign function and cast child component to parent Component_t
  pProgressBar->base.draw   = (void(*)(Component_t*))drawProgressBarComponent; 
  pProgressBar->base.update = (void(*)(Component_t*, void*))updateProgressBarComponent; 
  pProgressBar->base.type   = UIC_COMPONENT_PROGRESSBAR;
  pProgressBar->base.pos.x  = x;
  pProgressBar->base.pos.y  = y;
  
  // Add the component to the page
  return e_UiC_addComponentToPage((Component_t*) pProgressBar, pPage);

}

UiC_ErrorType e_UiC_newMenuItem(Component_t_MenuItem* pMenuItem, Component_t_MenuList* pMenu, uint8_t x, uint8_t y, char* value)
{
  pMenuItem->base.draw = (void(*) (Component_t*))drawMenuItemComponent;
  pMenuItem->base.update = (void(*) (Component_t*, void*))updateMenuItemComponent;
  pMenuItem->base.type   = UIC_COMPONENT_MENU_ITEM;
  pMenuItem->base.pos.x  = x;
  pMenuItem->base.pos.y  = y;
  strncpy(pMenuItem->itemText, value, sizeof(pMenuItem->itemText));

  return e_UiC_addMenuItemToMenu(pMenuItem, pMenu);
}

UiC_ErrorType e_UiC_newMenu(Component_t_MenuList* pMenu, Page_t* pPage)
{
  pMenu->base.draw   = (void(*) (Component_t*))drawMenuListComponent;
  pMenu->base.update =(void(*) (Component_t*, void*))updateMenuListComponent;
  pMenu->base.type   = UIC_COMPONENT_MENU_LIST;

  return e_UiC_addComponentToPage((Component_t*) pMenu, pPage);
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

static void updateMenuListComponent(Component_t_MenuList* pMenu, bool* newState)
{
  uint8_t i;
  for(i = 0; i < pMenu->nItems; i++)
  {
    pMenu->menuItems[i]->base.update((Component_t*) pMenu->menuItems[i], (void*) newState);
  }


}
