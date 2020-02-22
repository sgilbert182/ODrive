/*******************************************************************************
* File          : SubscriptionTools.cpp
*
* Description   : 
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 19 Feb 2020
*
*******************************************************************************/

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "SubscriptionTools.hpp"
#include "stddef.h"

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
CONSTANTS
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
MODULE VARIABLES
*******************************************************************************/

/*******************************************************************************
MODULE FUNCTION DECLARATIONS
*******************************************************************************/

/*******************************************************************************
FUNCTION DEFINITIONS
*******************************************************************************/

/**\brief   Constructor.
 *
 * \param   pTable      - pointer to the subscription table space
 * \param   maxEntries  - size of table space
 *
 * \return  returns pointer to active subscription or nullptr if not found
 */
CSubscribeBase::CSubscribeBase(void * pTable, size_t maxEntries)
    : m_pTable((subscription_t *)pTable)
    , m_maxEntries(maxEntries)
    , m_subscriptionCtr(0)
{
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore);
}

/**\brief   Checks to see if the GPIO details have already got an active
 *          subscription and returns the pointer.
 *
 * \param   GPIO_port   - pointer to the port struct
 * \param   GPIO_pin    - pin ID
 *
 * \return  returns pointer to active subscription or nullptr if not found
 */
CSubscribeBase::subscription_t * CSubscribeBase::findActiveSubscription(GPIO_TypeDef* GPIO_port
                                                            , uint16_t GPIO_pin)
{
    subscription_t * returnVal = nullptr;

    for (size_t i = 0; i < m_subscriptionCtr; ++i)
    {
        subscription_t * pSubscription = &m_pTable[i];
        if ((pSubscription->GPIO_port == GPIO_port)
                && (pSubscription->GPIO_InitStruct.Pin == GPIO_pin))
        {
            returnVal = pSubscription;
            break;
        }
    }

    return returnVal;
}

/**\brief   Registers a callback to the specified port and pin combination
 *
 * \param   GPIO_port       - pointer to the port struct
 * \param   GPIO_pin        - pin ID
 * \param   pull_up_down    - one of GPIO_NOPULL, GPIO_PULLUP or GPIO_PULLDOWN
 * \param   callback        - pointer to function to use for callback
 * \param   ctx             - pointer to callback argument
 *
 * \return  returns true if successful otherwise returns false
 */
bool CSubscribeBase::subscribe(GPIO_TypeDef * GPIO_port
                           , uint16_t GPIO_pin
                           , uint32_t pull_up_down
                           , callbackFuncPtr_t callback
                           , void * ctx)
{
    bool returnVal = false;

    if (xSemaphoreTake(xSemaphore, portMAX_DELAY))
    {
        // check for a pre-configured subscription for the port and pin
        subscription_t * subscription = findActiveSubscription(GPIO_port, GPIO_pin);

        // if one is not found then assign the next available one
        if (nullptr == subscription)
        {
            if (m_subscriptionCtr < m_maxEntries)
            {
                subscription = &m_pTable[m_subscriptionCtr];
                ++m_subscriptionCtr;
            }
        }

        // if one has been made available
        if (nullptr != subscription)
        {
            subscription->GPIO_port = GPIO_port;
            subscription->GPIO_InitStruct.Pin = GPIO_pin;
            subscription->GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
            subscription->GPIO_InitStruct.Pull = pull_up_down;
            subscription->callback = callback;
            subscription->ctx = ctx;

            configureGPIO(subscription);
            returnVal = true;
        }
        xSemaphoreGive(xSemaphore);
    }

    return returnVal;
}

/**\brief   Unregisters the callback of the specified port and pin combination
 *
 * \param   GPIO_port   - pointer to the port struct
 * \param   GPIO_pin    - pin ID
 *
 * \return  None
 */
void CSubscribeBase::unsubscribe(GPIO_TypeDef * GPIO_port, uint16_t GPIO_pin)
{
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY))
    {
        bool is_pin_in_use = false;
        uint32_t subscribeID = 0;

        for (size_t i = 0; i < m_subscriptionCtr; ++i)
        {
            subscription_t * pSubscription = &m_pTable[i];
            // if there is a complete match then clear entries
            if ((pSubscription->GPIO_port == GPIO_port)
                    && (pSubscription->GPIO_InitStruct.Pin == GPIO_pin))
            {
                pSubscription->callback = NULL;
                pSubscription->ctx = NULL;
                subscribeID = i;
            }
            else
            {
                // if there is only a pin match, we don't want to disable the interrupt
                if (pSubscription->GPIO_InitStruct.Pin == GPIO_pin)
                {
                    is_pin_in_use = true;
                }
            }
        }

        if (!is_pin_in_use)
        {
            unconfigureGPIO(&m_pTable[subscribeID]);
        }
        xSemaphoreGive(xSemaphore);
    }
}

/**\brief   Gets pointer to subscription list.
 *
 * \param   None
 *
 * \return  None
 */
bool CSubscribeBase::getSubscriptionList(uint32_t tableID, GPIO_TypeDef * GPIO_port, uint16_t * GPIO_pin)
{
    GPIO_port = m_pTable[tableID].GPIO_port;
    *GPIO_pin = m_pTable[tableID].GPIO_InitStruct.Pin;

    return true;
}

/**\brief   Gets active count on the subscription list.
 *
 * \param   None
 *
 * \return  None
 */
size_t CSubscribeBase::getSubscriptionCount(void)
{
    return m_subscriptionCtr;
}

/**\brief   Gets pointer to callback function.
 *
 * \param   listID  - subscription entry ID
 *
 * \return  None
 */
callbackFuncPtr_t CSubscribeBase::getCallback(uint32_t listID)
{
    return m_pTable[listID].callback;
}

/**\brief   Constructor.
 *
 * \param   pTable      - pointer to the subscription table space
 * \param   maxEntries  - size of table space
 *
 * \return  returns pointer to active subscription or nullptr if not found
 */
CSubscribeEXTI::CSubscribeEXTI(void * pTable, size_t maxEntries)
    : CSubscribeBase(pTable, maxEntries)
{}

/**\brief   Configures GPIO for external interrupt.
 *
 * \param   pSubscription   - pointer to struct containing pin assignment
 *
 * \return  None
 */
void CSubscribeEXTI::configureGPIO(subscription_t * pSubscription)
{
    // Set up GPIO
    HAL_GPIO_Init(pSubscription->GPIO_port, &pSubscription->GPIO_InitStruct);
    // Clear any previous triggers
    __HAL_GPIO_EXTI_CLEAR_IT(pSubscription->GPIO_InitStruct.Pin);
    // Enable interrupt
    HAL_NVIC_SetPriority(getIRQNumber(pSubscription->GPIO_InitStruct.Pin), 0, 0);
    HAL_NVIC_EnableIRQ(getIRQNumber(pSubscription->GPIO_InitStruct.Pin));
}

/**\brief   Disables the external interrupt for the pin.
 *
 * \param   pSubscription   - pointer to struct containing pin assignment
 *
 * \return  None
 */
void CSubscribeEXTI::unconfigureGPIO(subscription_t * pSubscription)
{
    HAL_NVIC_DisableIRQ(getIRQNumber(pSubscription->GPIO_InitStruct.Pin));
}

/**\brief   Returns the IRQ number associated with a certain pin. Note that all
 *          GPIOs with the same pin number map to the same IRQn, no matter which
 *          port they belong to.
 *
 * \param   pin     - pin ID
 *
 * \return  returns IRQ number
 */
IRQn_Type CSubscribeEXTI::getIRQNumber(uint16_t pin)
{
    IRQn_Type returnVal = (IRQn_Type)0;
    uint16_t pin_number = 0;
    uint16_t pinID = (pin >> 1);

    while (pinID)
    {
        pinID >>= 1;
        ++pin_number;
    }

    switch (pin_number)
    {
        case 0: returnVal = EXTI0_IRQn; break;
        case 1: returnVal = EXTI1_IRQn; break;
        case 2: returnVal = EXTI2_IRQn; break;
        case 3: returnVal = EXTI3_IRQn; break;
        case 4: returnVal = EXTI4_IRQn; break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9: returnVal = EXTI9_5_IRQn; break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15: returnVal = EXTI15_10_IRQn; break;
        default: returnVal = (IRQn_Type)0; break; // impossible fixme this 'id' represents the watchdog ISR
    }

    return returnVal;
}
