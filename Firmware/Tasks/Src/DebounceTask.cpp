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
#include "stm32f405xx.h"
#include "stdint.h"

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
CSubscribeDebounce::CSubscribeDebounce(void * pTable, size_t maxEntries)
    : CSubscribeBase(pTable, maxEntries)
{}

/**\brief   Configures GPIO for de-bouncing.
 *
 * \param   pSubscription   - pointer to struct containing pin assignment
 *
 * \return  None
 */
void CSubscribeDebounce::configureGPIO(subscription_t * pSubscription)
{
    // Set up GPIO
    HAL_GPIO_DeInit(pSubscription->GPIO_port, pSubscription->GPIO_InitStruct.Pin);
    HAL_GPIO_Init(pSubscription->GPIO_port, &pSubscription->GPIO_InitStruct);
    // Clear any previous triggers
    __HAL_GPIO_EXTI_CLEAR_IT(pSubscription->GPIO_InitStruct.Pin);
}

/**\brief   Removes the pin from the debounce list.
 *
 * \param   pSubscription   - pointer to struct containing pin assignment
 *
 * \return  None
 */
void CSubscribeDebounce::unconfigureGPIO(subscription_t * pSubscription)
{
    HAL_GPIO_DeInit(pSubscription->GPIO_port, pSubscription->GPIO_InitStruct.Pin);
}

/**\brief   Constructor.
 *
 * \param   OSThreadDef - thread config struct
 * \param   period      - task period in ms
 *
 * \return  None
 */
CDebounceTask::CDebounceTask(osThreadDef_t OSThreadDef, uint32_t period
                             , void * m_pSubscriptions
                             , size_t maxEntries)
    : m_OSThreadDef(OSThreadDef)
    , m_threadID(nullptr)
    , m_period(period)
    , GPIOData()
    , m_subscribedGPIOs(m_pSubscriptions, maxEntries)
{}

/**\brief   Registers a callback to the specified port and pin combination.
 *
 * \param   GPIO_port       - pointer to the port struct
 * \param   GPIO_pin        - pin ID
 * \param   pull_up_down    - one of GPIO_NOPULL, GPIO_PULLUP or GPIO_PULLDOWN
 * \param   callback        - pointer to function to use for callback
 * \param   ctx             - pointer to callback argument
 *
 * \return  returns true if successful otherwise returns false
 */
bool CDebounceTask::subscribe(GPIO_TypeDef * GPIO_port
                              , uint16_t GPIO_pin
                              , uint32_t pull_up_down
                              , callbackFuncPtr_t callback
                              , void * ctx)
{
    return m_subscribedGPIOs.subscribe(GPIO_port
                                , GPIO_pin
                                , pull_up_down
                                , callback
                                , ctx);
}

/**\brief   main thread function.
 *
 * \param   ctx - argument passed in from scheduler
 *
 * \return  None
 */
void CDebounceTask::threadFunc(void * ctx)
{
    (void)ctx;

    for(;;)
    {
        auto subscribeCount = m_subscribedGPIOs.getSubscriptionCount();
        updateIOs(subscribeCount);
        checkDebouncedIOs(subscribeCount);
        osDelay(m_period);
    }
}

/**\brief   Gets the current IO states of all registered IOs and pushes values
 *          to the debounce arrays. Then updates debounce states.
 *
 * \param   subscribeCount  - number of entries in list
 *
 * \return  None
 */
void CDebounceTask::updateIOs(uint32_t subscribeCount)
{
    GPIO_TypeDef * GPIO_port = nullptr;
    uint16_t pGPIO_pin = 0;
    uint32_t pinList = 0;

    for(auto i = 0u; i < subscribeCount; ++i)
    {
        m_subscribedGPIOs.getSubscriptionList(subscribeCount, GPIO_port, &pGPIO_pin);
        pinList |= (HAL_GPIO_ReadPin(GPIO_port, pGPIO_pin) << i);
    }

    GPIOData.update(pinList);
}

/**\brief   Checks the debounced IOs for an assert and calls the associated
 *          callback.
 *
 * \param   subscribeCount  - number of entries in list
 *
 * \return  None
 */
void CDebounceTask::checkDebouncedIOs(uint32_t subscribeCount)
{
    for(auto i = 0u; i < subscribeCount; ++i)
    {
        /* isAsserted() is true only when the IO (after debouncing) initially
         * changes state. Will not be true again until the IO is first
         * de-asserted.
         */
        if(GPIOData.isAsserted(i))
        {
            callbackFuncPtr_t callback = m_subscribedGPIOs.getCallback(i);
            if(callback)
            {
                callback(nullptr);
            }
        }
    }
}

/**\brief   Has selected GPIO been asserted.
 *
 * \param   GPIOID  - GPIO to be checked
 *
 * \return  returns true if the selected GPIO has been pressed
 */
bool CDebounceTask::isAsserted(uint32_t GPIOID)
{
    return GPIOData.isAsserted(GPIOID);
}

/**\brief   main thread function. Allows call of Virtual main function of child
 *          function.
 *
 * \param   pArg    - pointer to the created class, passed in from scheduler
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
