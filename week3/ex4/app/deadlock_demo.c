/* Week 3 / Ex4 -- task_alpha and task_beta share two resources, protected by
 * two mutexes, mimicking a sensor register + a shared UART buffer that both
 * tasks occasionally need at once. This file is provided complete, not a
 * skeleton: build and run it exactly as given first and use
 * uxTaskGetSystemState() to find out why it stops making progress, before
 * you change anything in here. Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "deadlock_demo.h"
#include "supervisor.h"
#include "runtime_stats.h"

static SemaphoreHandle_t s_mutex_a;
static SemaphoreHandle_t s_mutex_b;
static uint8_t one_slow = 1;        // Dùng cho report ques6

void deadlock_demo_init(void)
{
    s_mutex_a = xSemaphoreCreateMutex();
    s_mutex_b = xSemaphoreCreateMutex();
    configASSERT(s_mutex_a != NULL);
    configASSERT(s_mutex_b != NULL);
}

void task_alpha(void *argument)
{
    (void)argument;

    for (;;) {
        xSemaphoreTake(s_mutex_a, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(5)); /* simulated register access time */
        xSemaphoreTake(s_mutex_b, portMAX_DELAY);

        /* ... shared work using both resources ... */

        xSemaphoreGive(s_mutex_b);
        xSemaphoreGive(s_mutex_a);

        if (one_slow) {
            one_slow = 0;
            Uart_printf("task_alpha slow 1 time\r\n");
            HAL_Delay(500);
        }

        /* TODO (step 4 of README, after the deadlock is fixed): call
         *       supervisor_checkin(SUPERVISOR_BIT_ALPHA) here, AFTER both
         *       mutexes are released -- that is what makes the check-in mean
         *       "this task completed a full cycle", not just "this task is
         *       still being scheduled". */
        supervisor_checkin(SUPERVISOR_BIT_ALPHA);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task_beta(void *argument)
{
    (void)argument;

    for (;;) {
        xSemaphoreTake(s_mutex_a, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(5)); /* simulated UART buffer access time */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);  // check reset
        xSemaphoreTake(s_mutex_b, portMAX_DELAY);

        /* ... shared work using both resources ... */

        xSemaphoreGive(s_mutex_a);
        xSemaphoreGive(s_mutex_b);

        /* TODO (step 4 of README, after the deadlock is fixed): call
         *       supervisor_checkin(SUPERVISOR_BIT_BETA) here, AFTER both
         *       mutexes are released. */
        supervisor_checkin(SUPERVISOR_BIT_BETA);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

SemaphoreHandle_t deadlock_demo_mutex_a(void) { return s_mutex_a; }
SemaphoreHandle_t deadlock_demo_mutex_b(void) { return s_mutex_b; }