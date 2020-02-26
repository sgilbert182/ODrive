/*******************************************************************************
* File          : GPIODebounceTools.hpp
*
* Description   : GPIO debounce tools
*
* Project       :
*
* Author        : Simon Gilbert
*
* Created on    : 19 Feb 2020
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GPIODEBOUNCETOOLS_HPP
#define GPIODEBOUNCETOOLS_HPP

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "CircularBuffer.hpp"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

constexpr uint32_t DEBOUNCEWINDOW = 10;

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

class CGPIOData
{
public:
    CGPIOData(void);
    ~CGPIOData(void) = default;
    void update(uint32_t newState);
    void debounce(void);
    bool isAsserted(uint32_t GPIOID);
    bool isDeAsserted(uint32_t GPIOID);
    bool GPIODebouncedState(uint32_t GPIOID);
    uint32_t anyChangedState(void);

private:
    uint32_t m_stateArray[DEBOUNCEWINDOW];
    CCBBuffer<uint32_t> m_stateCB;
    uint32_t m_state;
    uint32_t m_changed;
};

/*******************************************************************************
FUNCTION PROTOTYPES
*******************************************************************************/

/*******************************************************************************
INLINE FUNCTIONS
*******************************************************************************/

#endif /* GPIODEBOUNCETOOLS_HPP */
