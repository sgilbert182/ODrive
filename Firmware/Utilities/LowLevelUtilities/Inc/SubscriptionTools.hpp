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
      uint16_t GPIO_pin;
      void (*callback)(void*);
      void* ctx;
    } subscription_t;

public:
    CSubscribeBase();
    virtual ~CSubscribeBase();
    bool subscribe(GPIO_TypeDef* GPIO_port, uint16_t GPIO_pin,
                   uint32_t pull_up_down,
                   void (*callback)(void*), void* ctx);
    void unsubscribe(GPIO_TypeDef* GPIO_port, uint16_t GPIO_pin);
    subscription_t const * const getSubscriptionList(void);
    size_t getSubscriptionCount(void);
    void * getCallback(uint32_t listID);

private:
    subscription_t * findActiveSubscription(GPIO_TypeDef* GPIO_port
                                            , uint16_t GPIO_pin);
    virtual void configureGPIO(subscription_t * subscription) = 0;
    virtual void unconfigureGPIO(subscription_t * subscription) = 0;

private:
    subscription_t subscriptions[MAX_SUBSCRIPTIONS] = { 0 };
    size_t n_subscriptions = 0;
};

class CSubscribeEXTI
    : public CSubscribeBase
{
private:
    void configureGPIO(subscription_t * subscription) override;
    void unconfigureGPIO(subscription_t * subscription) override;
    IRQn_Type getIRQNumber(uint16_t pin);
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

#endif /* SUBSCRIPTIONTOOLS_HPP */
