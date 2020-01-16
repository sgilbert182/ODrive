/*******************************************************************************
* File          : UARTControlTask.hpp
*
* Description   : UART receive and transmit handling task
*
* Project       :
*
* Author        : S.Gilbert
*
* Created on    : 14 Jan 2019
*
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UARTCONTROLTASK_HPP
#define UARTCONTROLTASK_HPP

/*******************************************************************************
INCLUDES
*******************************************************************************/

#include "TaskBase.hpp"
#include "CircularBuffer.hpp"
#include "protocol.hpp"

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

#define UART_RX_BUFFER_SIZE 64

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

class CUARTControlTask
    : public threadCore::CTaskBase
{
public:
    CUARTControlTask(char const * const pTaskName
                       , const float freq
                       , void * const pStack
                       , const size_t stackSize
                       , StreamToPacketSegmenter * pStreamIn
                       , StreamSink & rStreamOut);
    ~CUARTControlTask() = default;
    void funcCall(void) override;
    void writeData(uint8_t * pData, size_t length);

private:

private:
    float m_freq;
    uint8_t m_RXBuffer[UART_RX_BUFFER_SIZE];
    CCBBuffer<uint8_t> m_RXCircularBuffer;
    StreamToPacketSegmenter * m_pStreamIn;
    StreamSink & m_rStreamOut;
};

/*******************************************************************************
FUNCTION PROTOTYPES
*******************************************************************************/

/*******************************************************************************
INLINE FUNCTIONS
*******************************************************************************/

#endif /* UARTCONTROLTASK_HPP */
