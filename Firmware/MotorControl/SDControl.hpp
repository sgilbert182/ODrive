/*
 * PDControl.hpp
 *
 *  Created on: 21 Jan 2020
 *      Author: s.gilbert
 */

#ifndef SDCONTROL_HPP
#define SDCONTROL_HPP

#include "gpio.h"

class CSDControl
{
public:
    typedef void (*pFunc_t)(void *);

    struct SDPins_t
    {
        uint16_t step_gpio_pin = 0;
        uint16_t dir_gpio_pin = 0;
    };

    CSDControl(Controller& controller, float countsPerStep)
    : m_controller(controller)
    , m_countsPerStep(countsPerStep)
    , step_gpio_pin(0)
    , dir_gpio_pin(0)
    , step_port_(get_gpio_port_by_pin(step_gpio_pin))
    , step_pin_(get_gpio_pin_by_pin(step_gpio_pin))
    , dir_port_(get_gpio_port_by_pin(dir_gpio_pin))
    , dir_pin_(get_gpio_pin_by_pin(dir_gpio_pin))
    , m_pCallback()
    {

    }
    ~CSDControl() = default;

    void setPins(SDPins_t * config);
    bool getActive();
    void decode_step_dir_pins();
    void setStepDirectionActive(bool active, pFunc_t pCallback = nullptr);
    GPIO_PinState getDirection();
//private:


    // step/direction interface
    void step_cb() {
        if (getActive()) {
            m_controller.move_incremental((getDirection() == GPIO_PIN_SET)
                                         ? m_countsPerStep : -m_countsPerStep, true);
        }
    };

    Controller& m_controller;
    float m_countsPerStep;
    bool step_dir_active_ = false; // auto enabled after calibration, based on config.enable_step_dir
    // updated from config in constructor, and on protocol hook
    uint16_t step_gpio_pin;
    uint16_t dir_gpio_pin;

    GPIO_TypeDef* step_port_;
    uint16_t step_pin_;
    GPIO_TypeDef* dir_port_;
    uint16_t dir_pin_;
    pFunc_t m_pCallback;
};

inline void CSDControl::setPins(SDPins_t * config)
{
    step_gpio_pin = config->step_gpio_pin;
    dir_gpio_pin = config->dir_gpio_pin;
}

inline bool CSDControl::getActive()
{
    return step_dir_active_;
}

inline void CSDControl::decode_step_dir_pins() {
    step_port_ = get_gpio_port_by_pin(step_gpio_pin);
    step_pin_ = get_gpio_pin_by_pin(step_gpio_pin);
    dir_port_ = get_gpio_port_by_pin(dir_gpio_pin);
    dir_pin_ = get_gpio_pin_by_pin(dir_gpio_pin);
}

inline void CSDControl::setStepDirectionActive(bool active, pFunc_t pCallback) {
    if (active) {
        // Set up the direction GPIO as input
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = dir_pin_;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(dir_port_, &GPIO_InitStruct);
        m_pCallback = pCallback;

        // Subscribe to rising edges of the step GPIO
        GPIO_subscribe(step_port_, step_pin_, GPIO_PULLDOWN, m_pCallback, this);

        step_dir_active_ = true;
    } else {
        step_dir_active_ = false;

        // Unsubscribe from step GPIO
        GPIO_unsubscribe(step_port_, step_pin_);
    }
}

inline GPIO_PinState CSDControl::getDirection()
{
    return HAL_GPIO_ReadPin(dir_port_, dir_pin_);
}

#endif /* SDCONTROL_HPP */
