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

/**\brief   Constructor
 *
 * \param   pTaskName   - pointer to the task name
 * \param   freq        - task frequency. Set to zero for non periodic task
 * \param   pStack      - pointer to stack space
 * \param   stackSize   - stack size in bytes
 * \param   pStreamIn   - pointer to the input parser
 * \param   rStreamOut  - reference to the output stream handler
 *
 * \return  None
 */
CUARTControlTask::CUARTControlTask(char const * const pTaskName
                                         , const float freq
                                         , void * const pStack
                                         , const size_t stackSize
                                         , StreamToPacketSegmenter * pStreamIn
                                         , StreamSink & rStreamOut)
    : threadCore::CTaskBase(pTaskName, pStack, stackSize, nullptr)
    , m_freq(freq)
    , m_RXCircularBuffer(m_RXBuffer, ARRAY_LEN(m_RXBuffer), false, false)
    , m_pStreamIn(pStreamIn)
    , m_rStreamOut(rStreamOut)
{
    setTaskFrequency(m_freq);
}

/**\brief   Main thread function, this is called when the task resumes from a
 *          a suspend, and suspends the thread when it returns. Treat this as
 *          thread main.
 *          Wakes on receiving serial data, reads data out of circular buffer
 *          and sends it to the parser/handler.
 *
 * \param   None
 *
 * \return  None
 */
void CUARTControlTask::funcCall(void)
{
    uint8_t RXData[UART_RX_BUFFER_SIZE] = {0};
    size_t length = m_RXCircularBuffer.read(RXData, ARRAY_LEN(RXData));

    m_pStreamIn->process_bytes(RXData, length, nullptr); // TODO: use process_all
    ASCII_protocol_parse_stream(RXData, (uint16_t)length, m_rStreamOut);
}

/**\brief   writes data to the circular buffer and tells the task to wake.
 *
 * \param   pData       - pointer to the data to write to the circular buffer
 * \param   length      - number of characters to store
 *
 * \return  None
 */
void CUARTControlTask::writeData(uint8_t * pData, size_t length)
{
    (void)m_RXCircularBuffer.write(pData, length);
    this->resumeTaskFromISR();
}
