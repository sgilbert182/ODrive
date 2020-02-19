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

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "GPIODebounceTools.hpp"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
MODULE VARIABLES
*******************************************************************************/

/*******************************************************************************
INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/

/*******************************************************************************
FUNCTION IMPLEMENTATIONS
*******************************************************************************/

/**\brief       Constructor
 *
 * \param       debounceWindow  - length of the de-bounce array
 *
 * \return      None
 */
CGPIOData::CGPIOData(uint32_t debounceWindow)
    : m_debounceWindow(debounceWindow)
    , m_stateCB(m_stateArray, ARRAY_LEN(m_stateArray), false, true)
    , m_state(0)
{

}

/**\brief       adds new button state to buffer
 *
 * \param       newState        - new value to add to array
 *
 * \return      None
 */
void CGPIOData::update(uint32_t newState)
{
    m_stateCB.write(newState);
    m_state = debounce();
}

/**\brief       button de-bounce task
 *
 * \param       None
 *
 * \return      returns all de-bounced button states
 */
uint32_t CGPIOData::debounce(void)
{
    uint32_t returnVal = 0xffff;

    for(auto i = 0; i < m_debounceWindow; ++i)
    {
        returnVal &= (uint32_t)m_stateArray[i];
    }

    return returnVal;
}

/**\brief       Has selected button been pressed
 *
 * \param       button  - button to be checked
 *
 * \return      returns true if the selected button has been pressed
 */
bool CGPIOData::isPressed(uint32_t button)
{
    return (0u < (m_state & button));
}

/**\brief       Has any button been pressed
 *
 * \param       None
 *
 * \return      returns true if any button has been pressed
 */
bool CGPIOData::anyPressed(void)
{
    return (0u < (m_state & 0xffff));
}

