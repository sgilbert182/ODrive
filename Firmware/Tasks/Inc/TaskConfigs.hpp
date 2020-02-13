/*******************************************************************************
* File          : TaskConfigs.hpp
*
* Description   : 
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 10 Feb 2020
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TASKCONFIGS_HPP
#define TASKCONFIGS_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include <stdint.h>
#include <cmsis_os.h>

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

/*******************************************************************************
TYPES
*******************************************************************************/

typedef struct
{
    char const * pThreadName;
    uint32_t stackSize;
    osPriority priority;
}taskStruct_t;

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
CONSTANTS
*******************************************************************************/

constexpr taskStruct_t cmd_parse        {"CMDParser",           8096, osPriorityNormal};
constexpr taskStruct_t UART_server      {"UARTServer",          1024, osPriorityNormal};
constexpr taskStruct_t USB_server       {"USBServer",           1024, osPriorityNormal};
constexpr taskStruct_t Axis_thread      {"AxisServer",          2048, osPriorityNormal};
constexpr taskStruct_t Analog_server    {"AnalogServer",        2048, osPriorityLow};

/*******************************************************************************
NAMESPACE
*******************************************************************************/

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/


#endif /* TASKCONFIGS_HPP */