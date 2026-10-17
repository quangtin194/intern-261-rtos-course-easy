/* Week 1 / Ex4 — FreeRTOS hooks.
 *
 * Both hooks must be observable from outside the chip. A breakpoint is not a
 * deliverable: you need a UART line or a distinct LED pattern you can photograph.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include <string.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* TODO 1: report xTask / pcTaskName out of the chip, then stop.
     *
     * Careful: the stack of the offending task is already corrupt. Think about
     * what you are allowed to call here, and write the reasoning in REPORT.md.
     * Hint: this hook runs from the scheduler's context, not the task's.
     */
    (void)xTask;

    const char msg[] = "STACK OVERFLOW!, Task: ";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg) - 1, 100);

    if (pcTaskName != NULL) {
        HAL_UART_Transmit(&huart1, (uint8_t *)pcTaskName, strlen(pcTaskName), 100);
    }

    const char newline[] = "\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)newline, sizeof(newline) - 1, 100);

    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    /* TODO 2: report, then stop. */
    const char msg[] = "FREERTOS MALLOC FAILED!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg) - 1, 100);

    for (;;) {
    }
}

/* configSUPPORT_STATIC_ALLOCATION = 1 makes these two callbacks mandatory.
 * Provide the memory for the idle task and the timer task here. */
static StaticTask_t idleTaskTCB;                             // 76 bytes .bss
static StackType_t idleTaskStack[STATIC_TASK_STACK_WORDS];   // 128*4 = 512 bytes .bss

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* TODO 3: static buffers. Note the size is in WORDS, not bytes —
     *         getting this wrong is a silent overflow. */
    *ppxIdleTaskTCBBuffer = &idleTaskTCB;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = STATIC_TASK_STACK_WORDS;
}

static StaticTask_t timerTaskTCB;                            // 76 bytes .bss
static StackType_t timerTaskStack[STATIC_TASK_STACK_WORDS];  // 128*4 = 512 bytes .bss

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* TODO 4: same, for the software timer task. */
    *ppxTimerTaskTCBBuffer = &timerTaskTCB;
    *ppxTimerTaskStackBuffer = timerTaskStack;
    *pulTimerTaskStackSize = STATIC_TASK_STACK_WORDS;
}
