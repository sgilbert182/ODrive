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
#include <MotorControl/utils.h>
#include <fibre/protocol.hpp>
#include <cmsis_os.h>
#include <freertos_vars.h>

#include "TaskConfigs.hpp"

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

// FIXME: the stdlib doesn't know about CMSIS threads, so this is just a global variable
osThreadId uart_thread;

/*******************************************************************************
MODULE VARIABLES
*******************************************************************************/

static uint8_t RXBuffer[UART_RX_BUFFER_SIZE];
static CCBBuffer<uint8_t> RXCircularBuffer(RXBuffer, sizeof(RXBuffer), false, false);

UARTSender uart4_stream_output(&huart4);
StreamSink * uart4_stream_output_ptr = &uart4_stream_output;
StreamBasedPacketSink uart4_packet_output(uart4_stream_output);
BidirectionalPacketBasedChannel uart4_channel(uart4_packet_output);
StreamToPacketSegmenter uart4_stream_input(uart4_channel);

/*******************************************************************************
INTERNAL FUNCTION DEFINTIONS
*******************************************************************************/

/*******************************************************************************
FUNCTION DECLARATIONS
*******************************************************************************/

/**\brief   sends processed bytes back out on the serial port.
 *
 * \param   buffer              - pointer to the data to write to the circular buffer
 * \param   length              - number of characters to store
 * \param   processed_bytes     - pointer to the variable that reports the number
 *                              of processed bytes
 *
 * \return  None
 */
int UARTSender::process_bytes(const uint8_t * buffer, size_t length, size_t * processed_bytes)
{
    auto written = 0u;

    while (written < length)
    {
        written += TXCircularBuffer.write(buffer, length);
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

/**\brief   gets amount of free space in buffer.
 *
 * \param   None
 *
 * \return  byte count of free space
 */
size_t UARTSender::get_free_space(void)
{
    return TXCircularBuffer.remainingSpace();
}

/**\brief   UART handle task.
 *
 * \param   ctx - not used
 *
 * \return  None
 */
static void uart_server_thread(void * ctx) {
    (void) ctx;

    for (;;) {
        uint8_t RXData[UART_RX_BUFFER_SIZE] = {0};
        size_t length = RXCircularBuffer.read(RXData, sizeof(RXData));

        uart4_stream_input.process_bytes(RXData, length, nullptr); // TODO: use process_all
        ASCII_protocol_parse_stream(RXData, length, uart4_stream_output);

        osThreadSuspend(uart_thread);
    };
}

/**\brief   starts the UART handle task and associated peripherals.
 *
 * \param   None
 *
 * \return  None
 */
void start_uart_server(void) {
    // DMA is set up to receive in a circular buffer forever.
    // We don't use interrupts to fetch the data directly, DMA transfers the data into a
    // circular buffer and signals the uart task to deal with the data. Data is then
    // taken out of the circular buffer into a parse buffer, controlled by a state machine

    static uint8_t dma_rx_buffer[2];
    HAL_UART_Receive_DMA(&huart4, dma_rx_buffer, sizeof(dma_rx_buffer));

    const osThreadDef_t os_thread_def_UART_server = {
            UART_server.pThreadName,
            uart_server_thread,
            UART_server.priority,
            0,
            UART_server.stackSize
    };

    // Start UART communication thread
    uart_thread = osThreadCreate(&os_thread_def_UART_server, NULL);
}

/**
  * @brief  UART error callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
 void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if(&huart4 == huart) {
        HAL_UART_AbortReceive(huart);
        HAL_UART_Receive_DMA(huart, huart->pTxBuffPtr, huart->TxXferSize);
    }
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    if(&huart4 == huart) {
        osSemaphoreRelease(sem_uart_dma);
    }
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
    if(&huart4 == huart) {
        RXCircularBuffer.write(&huart->pRxBuffPtr[0], 1);
        osThreadResume(uart_thread);
    }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(&huart4 == huart) {
        RXCircularBuffer.write(&huart->pRxBuffPtr[1], 1);
        osThreadResume(uart_thread);
    }
}
