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
NAMESPACE
*******************************************************************************/

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

/**\brief   Constructor
 *
 * \param   None
 *
 * \return  None
 */
CGPIOData::CGPIOData(void)
    : m_stateCB(m_stateArray, ARRAY_LEN(m_stateArray), false, true)
    , m_state(0)
{

}

/**\brief   adds new GPIO state to buffer
 *
 * \param   newState    - new value to add to array
 *
 * \return  None
 */
void CGPIOData::update(uint32_t newState)
{
    m_stateCB.write(newState);
    m_state = debounce();
}

/**\brief   GPIO de-bounce task
 *
 * \param   None
 *
 * \return  returns all de-bounced GPIO states
 */
uint32_t CGPIOData::debounce(void)
{
    uint32_t returnVal = 0xffff;

    for(auto i = 0u; i < ARRAY_LEN(m_stateArray); ++i)
    {
        returnVal &= m_stateArray[i];
    }

    return returnVal;
}

/**\brief   Has selected GPIO been asserted
 *
 * \param   GPIOID  - GPIO to be checked
 *
 * \return  returns true if the selected GPIO has been pressed
 */
bool CGPIOData::isAsserted(uint32_t GPIOID)
{
    return (0u < (m_state & GPIOID));
}

/**\brief   Has any GPIO been asserted
 *
 * \param   None
 *
 * \return  returns true if any GPIO has been pressed
 */
bool CGPIOData::anyAsserted(void)
{
    return (0u < (m_state & 0xffff));
}
