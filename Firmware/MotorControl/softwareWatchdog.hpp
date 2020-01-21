/*******************************************************************************
* File          : softwareWatchdog.hpp
*
* Description   :
*
* Project       :
*
* Author        : s.gilbert
*
* Created on    : 21 Jan 2020
*
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SOFTWAREWATCHDOG_HPP
#define SOFTWAREWATCHDOG_HPP

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "stdint.h"

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
NAMESPACE
*******************************************************************************/

class CSoftwareWatchdog{

public:

    CSoftwareWatchdog(float watchdogTimeout = 0.0f)
        : m_watchdogTimeout(watchdogTimeout)
        , m_enabled(false)
        , m_currentValue(0)
    {}
    constexpr uint32_t get_watchdog_reset() {
        return static_cast<uint32_t>(std::clamp<float>(m_watchdogTimeout, 0, UINT32_MAX / (current_meas_hz + 1)) * current_meas_hz);
    }

    // @brief Feed the watchdog to prevent watchdog timeouts.
    void watchdog_feed() {
        m_currentValue = get_watchdog_reset();
    }

    auto getTimeout(void)
    {
        return m_watchdogTimeout;
    }

    // @brief Check the watchdog timer for expiration. Also sets the watchdog error bit if expired.
    bool watchdog_check() {
        // reset value = 0 means watchdog disabled.
        if (!m_enabled) return true;
        if (get_watchdog_reset() == 0) return true;

        // explicit check here to ensure that we don't underflow back to UINT32_MAX
        if (m_currentValue > 0) {
                m_currentValue--;
            return true;
        } else {
            return false;
        }
    }

//private:
    float m_watchdogTimeout;                                          // [s] (0 disables watchdog)
    bool m_enabled;
    uint32_t m_currentValue;
};

/*******************************************************************************
INLINE FUNCTIONS
*******************************************************************************/

#endif /* SOFTWAREWATCHDOG_HPP */
