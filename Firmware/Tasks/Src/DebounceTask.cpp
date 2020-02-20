/*******************************************************************************
* File          : DebounceTask.cpp
*
* Description   : 
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 20 Feb 2020
*
*******************************************************************************/

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "DebounceTask.hpp"
#include "stm32f4xx_hal.h"

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

/**\brief       Configures GPIO for de-bouncing.
 *
 * \param       pSubscription   - pointer to struct containing pin assignment
 *
 * \return      None
 */
void CSubscribeDebounce::configureGPIO(subscription_t * subscription)
{
    // Set up GPIO
    HAL_GPIO_Init(subscription->GPIO_port, &subscription->GPIO_InitStruct);
    // Clear any previous triggers
    __HAL_GPIO_EXTI_CLEAR_IT(subscription->GPIO_InitStruct.Pin);
}

/**\brief       Removes the pin from the debounce list.
 *
 * \param       pSubscription   - pointer to struct containing pin assignment
 *
 * \return      None
 */
void CSubscribeDebounce::unconfigureGPIO(subscription_t * subscription)
{

}

CDebounceTask::CDebounceTask(osThreadDef_t OSThreadDef, uint32_t period)
    : m_OSThreadDef(OSThreadDef)
    , m_threadID(nullptr)
    , m_period(period)
    , GPIOData()
    , m_subscribedGPIOs()
{

}

bool CDebounceTask::subscribe(GPIO_TypeDef * GPIO_port
                              , uint16_t GPIO_pin
                              , uint32_t pull_up_down
                              , void (* callback)(void *)
                              , void * ctx)
{
    return m_subscribedGPIOs.subscribe(GPIO_port
                                , GPIO_pin
                                , pull_up_down
                                , callback
                                , ctx);
}

void CDebounceTask::threadFunc(void * ctx)
{
    (void)ctx;

    for(;;)
    {
        auto GPIOList = m_subscribedGPIOs.getSubscriptionList();
        auto subscribeCount = m_subscribedGPIOs.getSubscriptionCount();
        uint32_t pinList = 0;
        for(auto i = 0u; i < subscribeCount; ++i)
        {
            pinList |= (HAL_GPIO_ReadPin(GPIOList->GPIO_port, GPIOList->GPIO_pin) << i);
        }

        GPIOData.update(pinList);
        for(auto i = 0u; i < subscribeCount; ++i)
        {
            if(GPIOData.isAsserted(i))
            {
                void (* callback)(void *) = (void (*)(void *))m_subscribedGPIOs.getCallback(i);
                if(callback)
                {
                    callback(nullptr);
                }
            }
        }
        osDelay(m_period);
    }
}

/**\brief   main thread function. Allows call of Virtual main function of child
 *              function
 *
 * \param   pArg        - pointer to the created class
 *
 * \return  None
 */
void CDebounceTask::_ThdFunc(void * pArg)
{
    if(nullptr != pArg)
    {
        ((CDebounceTask*)pArg)->threadFunc(nullptr);
    }
}

/**\brief   starts the debounce task and associated peripherals.
 *
 * \param   None
 *
 * \return  None
 */
void CDebounceTask::start(void)
{
    m_OSThreadDef.pthread = (void (*)(void *))&this->_ThdFunc;
    m_threadID = osThreadCreate(&m_OSThreadDef, this);
}
