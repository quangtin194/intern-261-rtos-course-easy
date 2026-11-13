/* Week 3 / Ex4 -- run-time stats on DWT CYCCNT.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "supervisor.h"
#include "deadlock_demo.h"
#include <stdio.h>
#include <string.h>

void Uart_printf(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

void runtime_stats_timer_init(void)
{
    /* TODO 1: same DWT enable sequence as week1/ex2 dwt_init() (CoreDebug
     *         DEMCR TRCENA bit, zero DWT->CYCCNT, DWT->CTRL CYCCNTENA bit).
     *         If you already have week1/ex2's dwt_init(), the sequence is
     *         identical -- do not invent a second mechanism. */
    /* Enable access to DWT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    /* Reset cycle counter */
    DWT->CYCCNT = 0;
    /* Enable CYCCNT */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t runtime_stats_timer_value(void)
{
    /* TODO 2: return DWT->CYCCNT. This is portGET_RUN_TIME_COUNTER_VALUE()'s
     *         implementation -- wire it up in FreeRTOSConfig.deltas. */
    return DWT->CYCCNT;
}

void runtime_stats_task(void *argument)
{
    (void)argument;
    static char buf[512];
    static TaskStatus_t taskStatus[5];  // Dùng  cho report câu 2
    static char line[96];

    for (;;) {
        /* TODO 3: vTaskGetRunTimeStats(buf) and print buf over UART.
         *         Note: this walks every task's TCB -- it is itself not free,
         *         factor that into why STATS_WINDOW_MS should not be too
         *         short. */
        for (int i = 0; i < STATS_WINDOW_MS / 100; i++) {
            vTaskDelay(pdMS_TO_TICKS(100));
            supervisor_checkin(SUPERVISOR_BIT_STATS);
        }

        memset(buf, 0, sizeof(buf));
        vTaskGetRunTimeStats(buf);
        uxTaskGetSystemState(taskStatus, 5, NULL);
        
        Uart_printf("\r\n");

        Uart_printf("===== RUN-TIME STATISTICS =====\r\n");
        Uart_printf(buf);
        Uart_printf("================================\r\n");

        // Uart_printf("===== STATE & MUTEX STATUS =====\r\n");
        // for (int i = 0; i < 5; i++) {
        //     snprintf(line, sizeof(line), "%-8s state=%d rt=%lu\r\n",
        //              taskStatus[i].pcTaskName,
        //              (int)taskStatus[i].eCurrentState,
        //              (unsigned long)taskStatus[i].ulRunTimeCounter);
        //     Uart_printf(line);
        // }
        // TaskHandle_t h = xSemaphoreGetMutexHolder(deadlock_demo_mutex_a());
        // snprintf(line, sizeof(line), "Mutex A held by: %s\r\n",
        //          h ? pcTaskGetName(h) : "none");
        // Uart_printf(line);

        // h = xSemaphoreGetMutexHolder(deadlock_demo_mutex_b());
        // snprintf(line, sizeof(line), "Mutex B held by: %s\r\n",
        //          h ? pcTaskGetName(h) : "none");
        // Uart_printf(line);
        // Uart_printf("================================\r\n");
    }
}
