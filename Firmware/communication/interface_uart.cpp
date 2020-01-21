/*******************************************************************************
* File          : interface_uart.cpp
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

/******************************************************************************
INCLUDES
*******************************************************************************/

#include "interface_uart.h"
#include "ascii_protocol.hpp"
#include "CircularBuffer.hpp"
#include <MotorControl/utils.h>
#include <fibre/protocol.hpp>
#include <usart.h>
#include <cmsis_os.h>
#include <freertos_vars.h>
#include "UARTControlTask.hpp"


/*******************************************************************************
NAMESPACE
*******************************************************************************/

/*******************************************************************************
DEFINITIONS
*******************************************************************************/

#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 64

/*******************************************************************************
TYPES
*******************************************************************************/

/*******************************************************************************
GLOBAL VARIABLES
*******************************************************************************/

// FIXME: the stdlib doesn't know about CMSIS threads, so this is just a global variable
// static thread_local uint32_t deadline_ms = 0;
osThreadId uart_thread;
const uint32_t stack_size_uart_thread = 2048;  // Bytes

/*******************************************************************************
MODULE VARIABLES
*******************************************************************************/

/*******************************************************************************
INTERNAL FUNCTION DEFINTIONS
*******************************************************************************/

/*******************************************************************************
FUNCTION DECLARATIONS
*******************************************************************************/

class UARTSender
    : public StreamSink
{
public:
    UARTSender(UART_HandleTypeDef * pHuart)
        : m_pHuart(pHuart)
        , TXCircularBuffer(TXBuffer, ARRAY_LEN(TXBuffer))
    {}
    virtual ~UARTSender() = default;
    int process_bytes(const uint8_t* buffer, size_t length, size_t* processed_bytes)
    {
        for(auto written = 0u; written < length; written += TXCircularBuffer.write(buffer, length))
        {
            if(nullptr != processed_bytes)
            {
                    *processed_bytes = written;
            }
            // Loop to ensure all bytes get sent
            while (!TXCircularBuffer.isEmpty())
            {
                if (osSemaphoreWait(sem_uart_dma, PROTOCOL_SERVER_TIMEOUT_MS) != osOK)
                {
                    return -1;
                }
                else
                {
                    // transmit chunk
                    if (HAL_UART_Transmit_DMA(m_pHuart, tx_buf_, TXCircularBuffer.read(tx_buf_, sizeof(tx_buf_))) != HAL_OK)
                    {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    size_t get_free_space() { return TXCircularBuffer.remainingSpace(); }

private:
    UART_HandleTypeDef * m_pHuart;
    uint8_t tx_buf_[UART_TX_BUFFER_SIZE];
    uint8_t TXBuffer[UART_TX_BUFFER_SIZE];
    CCBBuffer<uint8_t> TXCircularBuffer;
} uart_stream_output(&huart4);

StreamBasedPacketSink uart4_packet_output(uart_stream_output);
BidirectionalPacketBasedChannel uart4_channel(uart4_packet_output);
StreamToPacketSegmenter uart4_stream_input(uart4_channel);

uint32_t UARTStack[stack_size_uart_thread / sizeof(uint32_t)];
CUARTControlTask UARTControl("UART Controller", 0, UARTStack, sizeof(UARTStack), &uart4_stream_input, uart_stream_output);

void start_uart_server()
{
    // DMA open loop continuous circular buffer
    static uint8_t dma_rx_buffer[2];

    // DMA is set up to receive in a circular buffer forever.
    // We don't use interrupts to fetch the data, instead we periodically read
    // data out of the circular buffer into a parse buffer, controlled by a state machine
    HAL_UART_Receive_DMA(&huart4, dma_rx_buffer, sizeof(dma_rx_buffer));

    uart_thread = UARTControl.getHandle();
    // Start UART communication thread
    UARTControl.start(osPriorityNormal);
}

/**
  * @brief  UART error callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
 void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(&huart4 == huart)
    {
        HAL_UART_AbortReceive(huart);
        HAL_UART_Receive_DMA(huart, huart->pRxBuffPtr, huart->RxXferSize);
    }
}

 /**
   * @brief  Tx Transfer completed callbacks.
   * @param  huart pointer to a UART_HandleTypeDef structure that contains
   *                the configuration information for the specified UART module.
   * @retval None
   */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if(&huart4 == huart)
    {
        osSemaphoreRelease(sem_uart_dma);
    }
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if(&huart4 == huart)
    {
        UARTControl.writeData(&huart->pRxBuffPtr[0], 1);
    }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(&huart4 == huart)
    {
        UARTControl.writeData(&huart->pRxBuffPtr[1], 1);
    }
}

