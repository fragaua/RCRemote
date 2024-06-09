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



static U8G2_SSD1306 DisplayHandle = U8G2_SSD1306(U8G2_R0, U8X8_PIN_NONE);
static UiCore_t  uiCoreContext;

void v_UiM_init()
{
  
  DisplayHandle.begin();
  DisplayHandle.clearDisplay();
  DisplayHandle.setFont(u8g2_font_smolfont_tf);
  Serial.println("Draw component");

}

void v_UiM_draw()
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

// void v_UiM_newComponent(uint8_t pageIdx, ComponentType t, ComponentPosition_t pos)
// {
//   Serial.println("Creating Component");
//   uint8_t componentIndex = uiCoreContext.pageList[pageIdx].nComponents;
//   uiCoreContext.pageList[pageIdx].componentList[componentIndex].draw = componentDrawFunctionLUT[t];
//   uiCoreContext.pageList[pageIdx].componentList[componentIndex].pos = pos;
//   uiCoreContext.pageList[pageIdx].componentList[componentIndex].value = 230; // debug
//   uiCoreContext.pageList[pageIdx].nComponents++;
//   Serial.println("Created Component");
// }

void v_UiM_newProgressBar(Component_t_ProgressBar* pProgressBar, uint8_t pageIdx, uint8_t x, uint8_t y, int value)
{
  uint8_t componentIndex = uiCoreContext.pageList[pageIdx]->nComponents;
  
  // Initialize component
  // Assign function and cast child component to parent Component_t
  pProgressBar->base.draw = (void(*)(Component_t*))drawAnalogMonitorComponent; 
  pProgressBar->base.pos.x = x;
  pProgressBar->base.pos.y = y;

  // Update the initial progress bar value
  pProgressBar->value = value;

  // Add the component to the page
  uiCoreContext.pageList[pageIdx]->componentList[componentIndex] = (Component_t*)pProgressBar;
  uiCoreContext.pageList[pageIdx]->nComponents++;
}


// void v_UiM_updateComponent(uint8_t pageIdx, uint8_t componentIdx, int value)
// {
//   uiCoreContext.pageList[pageIdx]->componentList[componentIdx].value = value;
// }

void v_UiM_newPage()
{
  uiCoreContext.nPages++;
}



static void drawAnalogMonitorComponent(Component_t_ProgressBar* pAnalogMonitor)
{
  DisplayHandle.drawFrame(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, 108, 6);
  DisplayHandle.drawBox(pAnalogMonitor->base.pos.x, pAnalogMonitor->base.pos.y, map(pAnalogMonitor->value, 0, 1023, 18, 108), 6);
}






