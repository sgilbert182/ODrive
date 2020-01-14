/*******************************************************************************
* File          : UARTControlTask.cpp
*
* Description   : UART receive and transmit handling task
*
* Project       :
*
* Author        : S.Gilbert
*
* Created on    : 14 Jan 2020
*
*******************************************************************************/

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "UARTControlTask.hpp"
#include "ascii_protocol.hpp"

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
GLOBAL VARIABLES
*******************************************************************************/

/*******************************************************************************
MODULE VARIABLES
*******************************************************************************/

/*******************************************************************************
INTERNAL FUNCTION DEFINTIONS
*******************************************************************************/

/*******************************************************************************
FUNCTION DECLARATIONS
*******************************************************************************/

CUARTControlTask::CUARTControlTask(char const * const pTaskName
                                         , const float freq
                                         , void * const pStack
                                         , const size_t stackSize
                                         , StreamToPacketSegmenter * pStreamIn
                                         , StreamSink & rStreamOut)
    : threadCore::CTaskBase(pTaskName, pStack, stackSize, nullptr)
    , m_freq(freq)
//    , m_RXBuffer(0)
    , m_RXCircularBuffer(m_RXBuffer, ARRAY_LEN(m_RXBuffer), false, false)
    , m_pStreamIn(pStreamIn)
    , m_rStreamOut(rStreamOut)
{
    setTaskFrequency(m_freq);
}

void CUARTControlTask::funcCall(void)
{
    uint8_t RXData[UART_RX_BUFFER_SIZE] = {0};
    size_t length = m_RXCircularBuffer.read(RXData, ARRAY_LEN(RXData));

    m_pStreamIn->process_bytes(RXData, length, nullptr); // TODO: use process_all
    ASCII_protocol_parse_stream(RXData, (uint16_t)length, m_rStreamOut);
}


void CUARTControlTask::writeData(uint8_t * pData, size_t length)
{
    m_RXCircularBuffer.write(pData, length);
    this->resumeTaskFromISR();
}
