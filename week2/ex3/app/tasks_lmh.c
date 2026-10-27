/* Week 2 / Ex3 — priority inversion vs priority inheritance.
 *
 * task_low  (lowest prio)  holds the shared resource for LOW_HOLD_MS.
 * task_mid  (middle prio)  never touches the resource; pure CPU-bound work,
 *                          its only job is to be ready and preemptible so it
 *                          can steal CPU from task_low while task_low holds
 *                          the resource task_high is waiting for.
 * task_high (highest prio) needs the resource every HIGH_PERIOD_MS and is
 *                          the one whose block time we measure.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"
#include <stm32f103xb.h>
#include "dwt.h"

static SemaphoreHandle_t s_resource;

void resource_init(void)
{
#if RESOURCE_MODE == 0
    /* TODO 1: xSemaphoreCreateBinary(), then give it once so it starts
     *         "available" (binary semaphores start empty). */
    s_resource = xSemaphoreCreateBinary();
    configASSERT(s_resource != NULL);
    xSemaphoreGive(s_resource);
#else
    s_resource = xQueueCreateMutex();
    configASSERT(s_resource != NULL);
#endif
}

void task_low(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO 3: take s_resource (portMAX_DELAY), toggle a GPIO marking
         *         "resource held", busy-wait LOW_HOLD_MS using the ex2 DWT
         *         helper or a HAL tick-based wait, lower the GPIO, give
         *         s_resource back. Then yield/delay briefly before looping —
         *         this task must not monopolize the CPU outside the hold. */
        if (xSemaphoreTake(s_resource, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
            dwt_busy_wait_us(LOW_HOLD_MS * 1000U);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);

            xSemaphoreGive(s_resource);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void task_mid(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO 4: toggle a GPIO, busy-wait MID_WORK_MS, lower the GPIO, then
         *         vTaskDelay a short period so it keeps becoming Ready
         *         periodically rather than running once and blocking
         *         forever. This task never touches s_resource. */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1);
        dwt_busy_wait_us(MID_WORK_MS * 1000U);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

void task_high(void *argument)
{
    (void)argument;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        /* TODO 5: toggle a GPIO high (marks "task_high wants the resource"),
         *         take s_resource (portMAX_DELAY), toggle a second GPIO
         *         transition (marks "task_high got it"), do a small amount
         *         of work, give s_resource back, lower the first GPIO.
         *         The gap between the two GPIO edges on the logic analyzer
         *         is your block time t1 - t0. */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);          // t0
        if (xSemaphoreTake(s_resource, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0);      // t1
            __NOP();
            __NOP();
            __NOP();

            xSemaphoreGive(s_resource);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(HIGH_PERIOD_MS));
    }
}
