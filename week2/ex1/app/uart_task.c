/* Week 2 / Ex1 — prints filtered samples to USART1 at UART_BAUD.
 * queue_filtered -> UART.
 *
 * This is also the task you deliberately starve in step 7 (temporarily set
 * UART_BAUD-equivalent hardware baud to 9600) to force queue_filtered to fill.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "pipeline.h"

void uart_task(void *argument)
{
    (void)argument;
    for (;;) {
        filtered_sample_t in;

        /* TODO 1: xQueueReceive(queue_filtered, &in, portMAX_DELAY). */
        if (xQueueReceive(queue_filtered, &in, portMAX_DELAY) != pdPASS) {
            continue;
        }

        /* TODO 2: format "seq,ax,ay,az\r\n" and HAL_UART_Transmit on USART1.
         *         Transmit time at UART_BAUD is the thing that makes this
         *         task slow relative to sensor_task/filter_task — say so in
         *         REPORT.md with the actual byte time you computed. */
        char buffer [64];
        int len = snprintf(buffer, sizeof(buffer), 
                            "%lu, %d, %d, %d\r\n", 
                            (unsigned long)in.seq, 
                            (int)in.ax, (int)in.ay, (int)in.az);

        if ((len > 0) && (len < sizeof(buffer))) {
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);
        }

        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);   // Ghi nhận chu kỳ từ sens -> UART
    }
}
