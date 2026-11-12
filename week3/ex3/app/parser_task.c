/* Week 3 / Ex3 -- command parser: rx_stream -> change demo_task's
 * period/priority/suspend state at runtime.
 *
 * Commands (each terminated by '\r', max MAX_CMD_LEN bytes):
 *   P<ms>\r   -- set demo_task period to <ms>
 *   S<prio>\r -- vTaskPrioritySet(demo_task, <prio>)
 *   T<0|1>\r  -- 0 = vTaskSuspend(demo_task), 1 = vTaskResume(demo_task)
 */
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include <stdlib.h>

static TaskHandle_t s_demo_task_handle;
static volatile uint32_t s_demo_period_ms = 200;

static void apply_command(const char *cmd, size_t len)
{
    /* TODO 1: switch on cmd[0]: 'P' -> parse decimal into s_demo_period_ms
     *         (demo_task reads this every loop, no restart needed);
     *         'S' -> parse decimal, vTaskPrioritySet(s_demo_task_handle, n);
     *         'T' -> cmd[1]=='0' ? vTaskSuspend(...) : vTaskResume(...).
     *         Ignore anything malformed -- do not crash on bad input. */
    if (len == 0) return;
    switch (cmd[0])
    {
    case 'P':
        if (len > 1 && cmd[1] >= '0' && cmd[1] <= '9') {
            char *endptr;
            uint32_t new_period = strtoul(&cmd[1], &endptr, 10);
            if (*endptr == '\0' || *endptr == '#') {
                s_demo_period_ms = new_period;
            }
        }
        break;
    case 'S':
        if (len > 1 && cmd[1] >= '0' && cmd[1] <= '9') {
            char *endptr;
            unsigned long n = strtoul(&cmd[1], &endptr, 10);
            if ((*endptr == '\0' || *endptr == '#') && n < configMAX_PRIORITIES) {
                vTaskPrioritySet(s_demo_task_handle, (UBaseType_t)n);
            }
        }
        break;
    case 'T':
        if (len > 1 && (cmd[2] == '\0' || cmd[2] == '#')) {
            if (cmd[1] == '0')      vTaskSuspend(s_demo_task_handle);
            else if (cmd[1] == '1') vTaskResume(s_demo_task_handle);
        }
        break;
    default:
        break;
    }
}

void parser_task(void *argument)
{
    (void)argument;
    char cmd[MAX_CMD_LEN];
    size_t idx = 0;
    uint8_t discarding = 0;

    for (;;) {
        uint8_t byte;

        /* TODO 2: xStreamBufferReceive(rx_stream, &byte, 1, portMAX_DELAY). */

        /* TODO 3: if byte == '\r', call apply_command(cmd, idx), idx = 0.
         *         else if idx < MAX_CMD_LEN - 1, cmd[idx++] = byte.
         *         else (overflow): discard the whole partial command, idx = 0
         *         -- explain in REPORT.md why silently truncating instead
         *         would be worse. */
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);  // check workload

        xStreamBufferReceive(rx_stream, &byte, 1, portMAX_DELAY);
        if (byte == '\r')
        {
            if (!discarding) {
                cmd[idx] = '\0';
                apply_command(cmd, idx);
            }
            idx = 0;
            discarding = 0;
        } else if (byte == '\n' || discarding) {
            /* bỏ qua */
        } else if (idx < MAX_CMD_LEN - 1)
        {
            cmd[idx++] = byte;
        } else {
            idx = 0;
            discarding = 1;
        }
        // HAL_Delay(1);  // check workload
    }
}

void demo_task(void *argument)
{
    (void)argument;
    s_demo_task_handle = xTaskGetCurrentTaskHandle();

    for (;;) {
        /* TODO 4: toggle a GPIO pin here. */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // HAL_Delay(s_demo_period_ms);
        vTaskDelay(pdMS_TO_TICKS(s_demo_period_ms));
    }
}
