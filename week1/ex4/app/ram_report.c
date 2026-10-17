/* Week 1 / Ex4 — print the live RAM picture over USART1.
 *
 * This is your measurement instrument. It must be honest: read the values from
 * FreeRTOS, never from a constant you typed in.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "ram_report.h"
#include "main.h"
#include "params.h"
#include <string.h>
#include <stdio.h>
#include "timers.h"

/* TODO 1: a blocking UART print helper. It is called from a task, so it may
 *         block — but say in REPORT.md what would break if you called it from
 *         the stack overflow hook instead. */
void UART_printSring(const char *data) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, strlen(data), 500);
}

void ram_report_task(void *argument)
{
    (void)argument;

    for (;;) {
        /* TODO 2: print, every 2 seconds:
         *   - xPortGetFreeHeapSize()
         *   - xPortGetMinimumEverFreeHeapSize()
         *   - uxTaskGetStackHighWaterMark() for: dyn_task, stat_task,
         *     ram_report_task, the idle task (xTaskGetIdleTaskHandle()),
         *     and the timer task (xTimerGetTimerDaemonTaskHandle()).
         *
         * High water mark is returned in WORDS. Print both words and bytes so
         * your report cannot silently mix the two units. */

        

        char buffer[128];

        /* 1. Report heap usage */
        snprintf(buffer, sizeof(buffer),
                 "\r\n========== RAM REPORT ==========\r\n"
                 "Free heap: %lu bytes\r\n"
                 "Minimum ever free heap: %lu bytes\r\n",
                 (unsigned long)xPortGetFreeHeapSize(),
                 (unsigned long)xPortGetMinimumEverFreeHeapSize());
        UART_printSring(buffer);

        /* 2. Get task handles */
        TaskHandle_t handles[] = {
            dyn_task_Handle,
            stat_task_Handle,
            ram_report_task_Handle,
            xTaskGetIdleTaskHandle(),
            xTimerGetTimerDaemonTaskHandle()
        };

        const char *task_names[] = {
            "dyn task",
            "stat task",
            "ram report task",
            "Idle task",
            "Timer task"
        };

        /* 3. Report stack high-water mark */
        for (size_t i = 0; i < sizeof(handles) / sizeof(handles[0]); i++) {
            if (handles[i] == NULL) {
                snprintf(buffer, sizeof(buffer),
                         "%s: handle NULL / task unavailable\r\n",
                         task_names[i]);
                UART_printSring(buffer);
                continue;
            }

            UBaseType_t hwm_words = uxTaskGetStackHighWaterMark(handles[i]);

            unsigned long hwm_bytes = (unsigned long)hwm_words * sizeof(StackType_t);

            snprintf(buffer, sizeof(buffer),
                     "%s HWM: %lu words, %lu bytes\r\n",
                     task_names[i],
                     (unsigned long)hwm_words,
                     hwm_bytes);
            UART_printSring(buffer);
        }

        UART_printSring("================================\r\n");
         
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
