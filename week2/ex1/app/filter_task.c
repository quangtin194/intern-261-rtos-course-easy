/* Week 2 / Ex1 — moving-average filter, window = FILTER_WINDOW samples.
 * queue_raw -> queue_filtered.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"
#include <stm32f103xb.h>

void filter_task(void *argument)
{
    (void)argument;

    /* TODO 1: ring buffers for ax/ay/az, size FILTER_WINDOW, plus running sums.
     *         Do NOT recompute the full sum every sample — that is the whole
     *         point of a moving-average window. */
    int16_t ax_buf[FILTER_WINDOW] = {0};
    int16_t ay_buf[FILTER_WINDOW] = {0};
    int16_t az_buf[FILTER_WINDOW] = {0};

    int32_t ax_sum = 0;
    int32_t ay_sum = 0;
    int32_t az_sum = 0;

    uint8_t index = 0;
    uint8_t count = 0;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1);    // Đo thời gian tích lũy: Bắt đầu tích lũy

    for (;;) {
        raw_sample_t in;

        /* TODO 2: xQueueReceive(queue_raw, &in, portMAX_DELAY). */
        /* TODO 3: push into the ring buffer, update running sums, compute the
         *         averaged ax/ay/az. */
        if (xQueueReceive(queue_raw, &in, portMAX_DELAY) != pdPASS) 
        {
            continue;
        }
        else if (count == FILTER_WINDOW)
        {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 0);  // Đo thời gian tích lũy: Kết thúc tích lũy (đủ)
            ax_sum -= ax_buf[index];
            ay_sum -= ay_buf[index];
            az_sum -= az_buf[index];
        }
        else count++;

        ax_buf[index] = in.ax;
        ay_buf[index] = in.ay;
        az_buf[index] = in.az;

        ax_sum += ax_buf[index];
        ay_sum += ay_buf[index];
        az_sum += az_buf[index];

        index = (index + 1U) % FILTER_WINDOW;


        filtered_sample_t out;
        out.seq = in.seq;
        /* TODO 4: fill out.ax/ay/az from the running average. */
        out.ax = (int16_t)(ax_sum / count);
        out.ay = (int16_t)(ay_sum / count);
        out.az = (int16_t)(az_sum / count);

        /* TODO 5: (queue_filtered, &out, <timeout>) — same question
         *         as sensor_task: what should happen here if uart_task is
         *         slow and queue_filtered is full? */
        if (xQueueSend(queue_filtered, &out, portMAX_DELAY) != pdPASS) { }
    }
}
