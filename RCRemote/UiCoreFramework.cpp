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

static void drawAnalogMonitorComponent(Component_t_ProgressBar* pAnalogMonitor);
static void updateAnalogMonitorComponent(Component_t_ProgressBar* pAnalogMonitor, uint16_t* value);



static U8G2_SSD1306 DisplayHandle = U8G2_SSD1306(U8G2_R0, U8X8_PIN_NONE);
static UiCore_t  uiCoreContext;

void v_UiC_init()
{
  
  DisplayHandle.begin();
  DisplayHandle.clearDisplay();
  DisplayHandle.setFont(u8g2_font_smolfont_tf);
  uiCoreContext.nPages = 0;
  uiCoreContext.currentPage = 0;
  Serial.println("UiC Initialized");

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
  pProgressBar->base.draw   = (void(*)(Component_t*))drawAnalogMonitorComponent; 
  pProgressBar->base.update = (void(*)(Component_t*, void*))updateAnalogMonitorComponent; 
  pProgressBar->base.type   = PROGRESSBAR;
  pProgressBar->base.pos.x  = x;
  pProgressBar->base.pos.y  = y;
  
  // Add the component to the page
  pPage->componentList[pPage->nComponents] = (Component_t*)pProgressBar;
  pPage->nComponents++; 
}
// TODO: Can we make a generic component add function? Draw and update functions could be fetched via
// LUT (like it was at a certain point) and the value could be responsability of the updateCOmponent function.
// Position values could easily be generic since every component already has it anyway




void v_UiC_updateComponent(Component_t* pComponent, void* pValue)
{
  pComponent->update(pComponent, pValue);
}

UiC_ErrorType e_UiC_newPage(Page_t* pPage)
{
  if(uiCoreContext.nPages > MAX_NUMBER_PAGES-1)
  {
    return UiC_ERROR;
  }
  pPage->nComponents = 0;
  uiCoreContext.pageList[uiCoreContext.nPages] = pPage;
  uiCoreContext.nPages++;
  return UiC_OK;
}



// TODO: Implement the various component draw functions. At some point, these can be easily inserted into a "Module Specific" file.
static void drawAnalogMonitorComponent(Component_t_ProgressBar* pAnalogMonitor)
{

  DisplayHandle.drawFrame(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, 108, 6);
  DisplayHandle.drawBox(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, map(pAnalogMonitor->value, 0, 1023, 108, 0), 6);
}

static void updateAnalogMonitorComponent(Component_t_ProgressBar* pAnalogMonitor, uint16_t* value)
{

  pAnalogMonitor->value = *value;
}






