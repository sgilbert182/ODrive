/*******************************************************************************
* File          : interface_uart.hpp
*
* Description   :
*
* Project       :
*
* Author        :
*
* Created on    :
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACE_UART_HPP
#define INTERFACE_UART_HPP

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "fibre/protocol.hpp"
#include <cmsis_os.h>
#include "CircularBuffer.hpp"
#include <usart.h>

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

#define UART_TX_BUFFER_SIZE 128
#define UART_RX_BUFFER_SIZE 128

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

extern StreamSink* uart4_stream_output_ptr;
extern osThreadId uart_thread;

/*******************************************************************************
GLOBAL FUNCTIONS
*******************************************************************************/

void start_uart_server(void);

/*******************************************************************************
CONSTANTS
*******************************************************************************/

/*******************************************************************************
NAMESPACE
*******************************************************************************/

class UARTSender
    : public StreamSink
{
public:
    UARTSender(UART_HandleTypeDef * pHuart)
        : m_pHuart(pHuart)
        , TXCircularBuffer(TXBuffer, ARRAY_LEN(TXBuffer))
    {}
    virtual ~UARTSender(void) = default;
    int process_bytes(const uint8_t * buffer, size_t length, size_t * processed_bytes);
    size_t get_free_space(void);

private:
    UART_HandleTypeDef * m_pHuart;
    uint8_t tx_buf_[UART_TX_BUFFER_SIZE];
    uint8_t TXBuffer[UART_TX_BUFFER_SIZE];
    CCBBuffer<uint8_t> TXCircularBuffer;
};

/*******************************************************************************
INLINE FUNCTION DEFINITIONS
*******************************************************************************/

#endif /* INTERFACE_UART_HPP */
