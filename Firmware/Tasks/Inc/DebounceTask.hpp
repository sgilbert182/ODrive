/*******************************************************************************
* File          : DebounceTask.hpp
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEBOUNCETASK_HPP
#define DEBOUNCETASK_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "cmsis_os.h"
#include "GPIODebounceTools.hpp"
#include "SubscriptionTools.hpp"

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
CONSTANTS
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

class CSubscribeDebounce
    : public CSubscribeBase
{
private:
    void configureGPIO(subscription_t * subscription) override;
    void unconfigureGPIO(subscription_t * subscription) override;
};

class CDebounceTask
{
public:
    CDebounceTask(osThreadDef_t OSThreadDef
                  , uint32_t period);
    ~CDebounceTask(void) = default;
    void start(void);
    void threadFunc(void * ctx);
    bool subscribe(GPIO_TypeDef * GPIO_port
                  , uint16_t GPIO_pin
                  , uint32_t pull_up_down
                  , void (* callback)(void *)
                  , void * ctx);

private:
    static void _ThdFunc(void * pArg);

private:
    osThreadDef_t m_OSThreadDef;
    osThreadId m_threadID;
    uint32_t m_period;
    CGPIOData GPIOData;
    CSubscribeDebounce m_subscribedGPIOs;
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

#endif /* DEBOUNCETASK_HPP */
