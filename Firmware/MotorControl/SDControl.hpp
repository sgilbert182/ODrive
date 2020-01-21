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

    CSDControl() = default;
    ~CSDControl() = default;

    auto getActive()
    {
        return step_dir_active_;
    }

    void decode_step_dir_pins();

    void setStepDirectionActive(bool active, pFunc_t pCallback = nullptr) {
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

    auto getDirection()
    {
        return HAL_GPIO_ReadPin(dir_port_, dir_pin_);
    }
//private:

    bool step_dir_active_ = false; // auto enabled after calibration, based on config.enable_step_dir
    // updated from config in constructor, and on protocol hook
    uint16_t step_gpio_pin = 0;
    uint16_t dir_gpio_pin = 0;

    GPIO_TypeDef* step_port_;
    uint16_t step_pin_;
    GPIO_TypeDef* dir_port_;
    uint16_t dir_pin_;
    pFunc_t m_pCallback;
};

inline void CSDControl::decode_step_dir_pins() {
    step_port_ = get_gpio_port_by_pin(step_gpio_pin);
    step_pin_ = get_gpio_pin_by_pin(step_gpio_pin);
    dir_port_ = get_gpio_port_by_pin(dir_gpio_pin);
    dir_pin_ = get_gpio_pin_by_pin(dir_gpio_pin);
}

#endif /* SDCONTROL_HPP */
