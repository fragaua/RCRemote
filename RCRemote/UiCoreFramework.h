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


// Tweaking these values will allow for more or less memory usage by the overall Ui Core and Management systems
#define MAX_COMPONENTS_PER_VIEW 18u
#define MAX_NUMBER_PAGES        6u
#define MAX_NR_CHARS            5u
#define MAX_NR_MENU_ITEMS       10u 


enum UiC_ErrorType
{
    UiC_OK,
    UiC_ERROR
};


enum ComponentType
{
    UIC_COMPONENT_TEXT,
    UIC_COMPONENT_ANALOGMONITOR,
    UIC_COMPONENT_ANALOGADJUSTMENT,
    UIC_COMPONENT_MENU_ITEM,
    UIC_COMPONENT_MENU_LIST,
    N_COMPONENT_TYPES // Last enum is essentially the total number of component types.
};


typedef struct Component_t_Position
{
    uint8_t x;
    uint8_t y;
}Component_t_Position;

// This structure contains all possible data needed to create, initialize and
// add a new component to a page.
typedef struct Component_t_Data // TODO: Change this name
{
    uint8_t x;
    uint8_t y;
    char*   stringData;
    void*   extraData;
}Component_t_Data;

// Component base. Every component child has a reference to this parent structure.
// The idea is to have some sort of class and inheritance in C
typedef struct Component_t
{
    ComponentType       type;
    Component_t_Position pos;
    void (*draw)  (Component_t* s);
    void (*update)(Component_t* s, void* v);
}Component_t;

typedef struct Component_t_Text
{
    Component_t base;
    char        value[MAX_NR_CHARS];
}Component_t_Text;

typedef struct Component_t_AnalogMonitor
{
    Component_t base;
    uint16_t    value;
}Component_t_AnalogMonitor;


// Placing the adjuster variable on the upper or lower 16bits of the 32bit variable will 
// make it update a different value
typedef struct Component_t_AnalogAdjustment
{
    Component_t base;
    uint16_t    value1;
    uint16_t    value2;
}Component_t_AnalogAdjustment; 

typedef struct Component_t_MenuItem
{
    Component_t base;
    char        itemText[MAX_NR_CHARS];
    // TODO: Add possibility to add an image to this menu item
    bool        isSelected;
    bool        isClicked;
    void        (*callback)(void* params);
}Component_t_MenuItem;

typedef struct Component_t_MenuList
{
    Component_t base;
    Component_t_MenuItem* menuItems[MAX_NR_MENU_ITEMS];
    uint8_t currentlySelectedIdx; // TODO: Better to have the index instead of ptr?
    uint8_t nItems;
    
}Component_t_MenuList;

typedef struct UiC_Input_t
{
    bool inputUp     : 1;
    bool inputDown   : 1;
    bool inputSelect : 1;
}UiC_Input_t;


typedef struct Page_t
{ 
    // This must be a pointer array since the elements we are adding are of different, 
    // bigger sizes than Component_t.
    Component_t* componentList[MAX_COMPONENTS_PER_VIEW]; 
    uint8_t      nComponents;
}Page_t;


// 17/06/2024 - Changed 'currentPage' from uint8 index to an actual pointer
// This allows us to switch to pages in an absolute fashion without having to
// have a 'fetch page' function like we had. 
// Analogous to this, I will change the MenuItem 'currentlySelected' to an index
// instead of a pointer, because for menus, I am more interested in having it
// be relatively selected to other items
typedef struct UiCore_t
{
    Page_t* pageList[MAX_NUMBER_PAGES];
    // uint8_t currentPage;
    Page_t* currentPage; 
    uint8_t nPages;

    UiC_ErrorType internalErrorState;
}UiCore_t;


/**  Core functionality **/
void v_UiC_init();
void v_UiC_draw();

/** Page Handling  **/
UiC_ErrorType e_UiC_newPage(Page_t* pPage);
void          v_UiC_changePage(Page_t* nextPage); 
Page_t*       UiC_getActivePage();

/** Component handling **/


/// @brief Generic Ui Core function to add components to a certain page. This takes care of initializing a component of type <eComponentType>
///        with all their base parameters as well potential optional parameters, passed through the <baseData> struct.
///        Some components have mandatory extra parameters, others have optional extra parameters while others have no extra parameters at all.
/// @param pComponent       The component to be initialized and added to page <pPage>
/// @param pPage            The page to which component <pComponent> will be added
/// @param eComponentType   The type of the component.
/// @param baseData         
/// @return 
UiC_ErrorType e_UiC_addComponent(Component_t* pComponent, Page_t* pPage, ComponentType eComponentType, Component_t_Data componentParameters);
void          v_UiC_updateComponent(Component_t* pComponent, void* pValue);


/** Error Handling **/
UiC_ErrorType UiC_getErrorState();

#endif;