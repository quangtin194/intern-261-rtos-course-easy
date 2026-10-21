/* Week 2 / Ex1 — sensor pipeline: sensor_task -> queue_raw -> filter_task ->
 * queue_filtered -> uart_task.
 *
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"
#include <stdio.h>
#include <string.h>

#define LOG_N 100   

QueueHandle_t queue_raw;
QueueHandle_t queue_filtered;
TaskHandle_t h_sensor, h_filter, h_uart;

typedef struct { uint16_t tick; uint8_t raw, filt; } depth_t;
static depth_t depth_log[LOG_N];

static void error_fatal(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    for (;;) {}
}

void depth_logger_task(void *argument)
{
    (void)argument;
    TickType_t t = xTaskGetTickCount();

    /* B1: Lấy mẫu độ đầy mỗi 1 ms vào mảng RAM */
    for (uint32_t i = 0; i < LOG_N; i++) {
        depth_log[i].tick = (uint16_t)xTaskGetTickCount();
        depth_log[i].raw  = (uint8_t)uxQueueMessagesWaiting(queue_raw);
        depth_log[i].filt = (uint8_t)uxQueueMessagesWaiting(queue_filtered);
        vTaskDelayUntil(&t, pdMS_TO_TICKS(1));
    }

    /* B2: Dừng sensor task, chờ pipeline xả */
    vTaskSuspend(h_sensor);

    while (uxQueueMessagesWaiting(queue_raw) != 0 ||
           uxQueueMessagesWaiting(queue_filtered) != 0 ||
           eTaskGetState(h_filter) != eBlocked ||
           eTaskGetState(h_uart)   != eBlocked)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /* B3: log UART */
    char line[32];
    for (uint32_t i = 0; i < LOG_N; i++) {
        int n = snprintf(line, sizeof(line), "D,%u,%u,%u\r\n",
                         (unsigned)depth_log[i].tick,
                         (unsigned)depth_log[i].raw,
                         (unsigned)depth_log[i].filt);
        if (n > 0) {
            HAL_UART_Transmit(&huart1, (uint8_t *)line, (uint16_t)n, HAL_MAX_DELAY);
        }
    }

    vTaskSuspend(NULL);
}

void app_main(void)
{
    /* TODO 1: xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t)) for queue_raw,
     *         and the equivalent for queue_filtered with filtered_sample_t.
     *         Check both handles are non-NULL before continuing. */
    queue_raw = xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t));
    queue_filtered = xQueueCreate(QUEUE_LENGTH, sizeof(filtered_sample_t));

    if (queue_raw == NULL) {
        error_fatal("Create raw queue failed\r\n");
    }
    if (queue_filtered == NULL) {
        error_fatal("Create filtered queue failed\r\n");
    }

    /* TODO 2: xTaskCreate sensor_task, filter_task, uart_task.
     *         Pick priorities you can justify in REPORT.md: who must never be
     *         starved by whom? */
    if (xTaskCreate(sensor_task, "Sensor task", 128U, NULL, 3, &h_sensor) != pdPASS) {
        error_fatal("Create sensor task failed\r\n");
    }
    if (xTaskCreate(filter_task, "Filter task", 128U, NULL, 2, &h_filter) != pdPASS) {
        error_fatal("Create filter task failed\r\n");
    }
    if (xTaskCreate(uart_task, "Uart task", 256U, NULL, 2, &h_uart) != pdPASS) {
        error_fatal("Create uart task failed\r\n");
    }

    /* TODO 3: xTaskCreate depth_logger_task — must not perturb the pipeline it
     *         is measuring (think about its own priority and period). */
    if (xTaskCreate(depth_logger_task, "Depth logger task", 256U, NULL, 4, NULL) != pdPASS) {
        error_fatal("Create depth logger task failed\r\n");
    }

    vTaskStartScheduler();

    for (;;) {
    }
}
