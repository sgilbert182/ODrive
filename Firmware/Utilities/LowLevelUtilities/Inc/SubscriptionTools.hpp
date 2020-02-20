/*******************************************************************************
* File          : SubscriptionTools.hpp
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SUBSCRIPTIONTOOLS_HPP
#define SUBSCRIPTIONTOOLS_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "stm32F405xx.h"
#include "stm32f4xx_hal.h"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

#define MAX_SUBSCRIPTIONS 10

/*******************************************************************************
TYPES
*******************************************************************************/

typedef void (* callbackFuncPtr_t)(void *);

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
CONSTANTS
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

/*
 *  base subscription class, hosts the subscription table and basic tools for
 *  registering, finding, and deregistering callbacks to pins
 */
class CSubscribeBase
{
public:
    typedef struct
    {
      GPIO_TypeDef * GPIO_port;
      GPIO_InitTypeDef GPIO_InitStruct;
      callbackFuncPtr_t callback;
      void* ctx;
    } subscription_t;

public:
    CSubscribeBase();
    virtual ~CSubscribeBase();
    bool subscribe(GPIO_TypeDef* GPIO_port, uint16_t GPIO_pin,
                   uint32_t pull_up_down,
                   callbackFuncPtr_t callback, void * ctx);
    void unsubscribe(GPIO_TypeDef* GPIO_port, uint16_t GPIO_pin);
    subscription_t const * const getSubscriptionList(void);
    size_t getSubscriptionCount(void);
    callbackFuncPtr_t getCallback(uint32_t listID);

private:
    subscription_t * findActiveSubscription(GPIO_TypeDef* GPIO_port
                                            , uint16_t GPIO_pin);
    virtual void configureGPIO(subscription_t * pSubscription) = 0;
    virtual void unconfigureGPIO(subscription_t * pSubscription) = 0;

private:
    subscription_t * m_pTable;
    size_t m_maxEntries;
    size_t m_subscriptionCtr;
};

class CSubscribeEXTI
    : public CSubscribeBase
{
private:
    void configureGPIO(subscription_t * pSubscription) override;
    void unconfigureGPIO(subscription_t * pSubscription) override;
    IRQn_Type getIRQNumber(uint16_t pin);
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

#endif /* SUBSCRIPTIONTOOLS_HPP */
