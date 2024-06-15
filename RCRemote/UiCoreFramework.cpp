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


/** Internal UiC functions **/
static UiC_ErrorType e_UiC_addComponentToPage(Component_t* pComponent, Page_t* pPage);


static U8G2_SSD1306 DisplayHandle = U8G2_SSD1306(U8G2_R0, U8X8_PIN_NONE);
static UiCore_t     uiCoreContext;

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
  
  // // Add the component to the page
  // pPage->componentList[pPage->nComponents] = (Component_t*)pText;
  // pPage->nComponents++; 
  return e_UiC_addComponentToPage((Component_t*) pText, pPage);

}



void v_UiC_updateComponent(Component_t* pComponent, void* pValue)
{
  pComponent->update(pComponent, pValue);
}

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





