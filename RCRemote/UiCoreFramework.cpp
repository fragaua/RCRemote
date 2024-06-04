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

static void drawAnalogMonitorComponent(uint8_t x, uint8_t y, int value);


static void (*componentDrawFunctionLUT[N_COMPONENT_TYPES])(uint8_t x, uint8_t y, int value) = 
{
  drawAnalogMonitorComponent,
  drawAnalogMonitorComponent,
  drawAnalogMonitorComponent
}; 


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
    Component_t currentComponent;
    for(i = 0; i < uiCoreContext.pageList[uiCoreContext.currentPage].nComponents; i++)
    {
      currentComponent = uiCoreContext.pageList[uiCoreContext.currentPage].componentList[i];
      currentComponent.draw(currentComponent.pos.x, currentComponent.pos.y, currentComponent.value);
    }

  }while(DisplayHandle.nextPage());

}

void v_UiM_newComponent(uint8_t pageIdx, ComponentType t, ComponentPosition_t pos)
{
  Serial.println("Creating Component");
  uint8_t componentIndex = uiCoreContext.pageList[pageIdx].nComponents;
  uiCoreContext.pageList[pageIdx].componentList[componentIndex].draw = componentDrawFunctionLUT[t];
  uiCoreContext.pageList[pageIdx].componentList[componentIndex].pos = pos;
  uiCoreContext.pageList[pageIdx].componentList[componentIndex].value = 230; // debug
  uiCoreContext.pageList[pageIdx].nComponents++;
  Serial.println("Created Component");
}
void v_UiM_updateComponent(uint8_t pageIdx, uint8_t componentIdx, int value)
{
  uiCoreContext.pageList[pageIdx].componentList[componentIdx].value = value;
}

void v_UiM_newPage()
{
  uiCoreContext.nPages++;
}



static void drawAnalogMonitorComponent(uint8_t x, uint8_t y, int value)
{
  DisplayHandle.drawFrame(x, y, 108, 6);
  DisplayHandle.drawBox(x, y, map(value, 0, 1023, 18, 108), 6);
}






