/**
 * @file UiCoreFramework.h 
 * @author Marcelo Fraga
 * @brief Header file for UiCoreFramework. This component aims to facilitate the creation
 * of a general purpose user interface. For this particular module, we are trying to use
 * some sort of "Object Oriented" approach, but only in C.
 * The idea is that this will allow for: Learning, Lower memory usage (in theory)...
 * @version 0.1
 * @date 2024 - 06 - 01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef UICOREFRAMEWORK_H
#define UICOREFRAMEWORK_H
#include <U8g2lib.h>
#include <Wire.h>
typedef U8G2_SSD1306_128X64_NONAME_1_HW_I2C U8G2_SSD1306;


#define MAX_COMPONENTS_PER_VIEW 10u
#define MAX_NUMBER_PAGES        4u
#define MAX_NR_CHARS            5u

enum ComponentType
{
    TEXT,
    IMAGE,
    ANALOGMONITOR,
    N_COMPONENT_TYPES // Last enum element is equal to the total nr of types
};

typedef struct ComponentPosition_t
{
    uint8_t x;
    uint8_t y;
}ComponentPosition_t;

// Component base. Every component child has a reference to this parent structure.
// The idea is to have some sort of inheritance
typedef struct Component_t
{
    ComponentPosition_t pos;
    void (*draw)(uint8_t x, uint8_t y, int value);
}Component_t;

typedef struct Page_t
{ 
    Component_t componentList[MAX_COMPONENTS_PER_VIEW];
    uint8_t nComponents;
}Page_t;

typedef struct Page_t_Inputs
{

}Page_t_Inputs;

typedef struct UiCore_t
{
    Page_t pageList[MAX_NUMBER_PAGES];
    uint8_t currentPage;
    uint8_t nPages;

    Page_t_Inputs inputs;
}UiCore_t;


void v_UiM_draw();
void v_UiM_init();
void v_UiM_newComponent(uint8_t pageIdx, ComponentType t, ComponentPosition_t pos);
void v_UiM_updateComponent(uint8_t pageIdx, uint8_t componentIdx, int value);
void v_UiM_newPage();


#endif;