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


enum UiC_ErrorType
{
    UiC_OK,
    UiC_ERROR
};


enum ComponentType
{
    PROGRESSBAR,
    TEXT,
    N_COMPONENTS // Last enum is essentially the total number of component types.
};


typedef struct ComponentPosition_t
{
    uint8_t x;
    uint8_t y;
}ComponentPosition_t;

// Component base. Every component child has a reference to this parent structure.
// The idea is to have some sort of class and inheritance in C
typedef struct Component_t
{
    ComponentType       type;
    ComponentPosition_t pos;
    void (*draw)  (Component_t* s);
    void (*update)(Component_t* s, void* v);
}Component_t;

typedef struct Component_t_Text
{
    Component_t base;
    char        value[MAX_NR_CHARS];  // TODO: Make sure we can pass strings from 

}Component_t_Text;

typedef struct Component_t_ProgressBar
{
    Component_t base;
    uint16_t    value;
}Component_t_ProgressBar;


typedef struct Page_t
{ 
    // This must be a pointer array since the elements we are adding are of different, 
    // bigger sizes than Component_t.
    Component_t* componentList[MAX_COMPONENTS_PER_VIEW]; 
    uint8_t nComponents;
}Page_t;

typedef struct Page_t_Inputs
{

}Page_t_Inputs;

typedef struct UiCore_t
{
    Page_t* pageList[MAX_NUMBER_PAGES];
    uint8_t currentPage;
    uint8_t nPages;

    Page_t_Inputs inputs;
}UiCore_t;


void v_UiC_draw();
void v_UiC_init();
void v_UiC_newText(Component_t_Text* pText, uint8_t pageIdx, uint8_t x, uint8_t y, const char* pContent);
UiC_ErrorType e_UiC_newProgressBar(Component_t_ProgressBar* pProgressBar, Page_t* pPage, uint8_t x, uint8_t y);
void v_UiC_updateComponent(Component_t* pComponent, void* pValue);
UiC_ErrorType e_UiC_newPage(Page_t* pPage);


#endif;