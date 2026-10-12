/* Week 1 / Ex2 — the three competing tasks.
 *
 * Each task: raise its pin, burn exactly TASK_x_WORK_US of CPU, lower its pin,
 * and loop again WITHOUT blocking. The missing block is the point of the
 * exercise, not a bug to fix in step 2.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

/* TODO: one helper that takes a port/pin and a work time, then three thin
 *       wrappers. Do not copy-paste the body three times. */
static void task_run (GPIO_TypeDef *GPIO_port, uint8_t GPIO_pin, uint32_t work_us) {
    HAL_GPIO_WritePin(GPIO_port, (uint16_t)(1U << GPIO_pin), GPIO_PIN_SET);
    dwt_busy_wait_us(work_us);
    HAL_GPIO_WritePin(GPIO_port, (uint16_t)(1U << GPIO_pin), GPIO_PIN_RESET);
}

void task_a(void *argument)
{
    (void)argument;
    for (;;) {
        task_run(GPIOA, TASK_A_PIN, TASK_A_WORK_US);
    }
}

void task_b(void *argument)
{
    (void)argument;
    for (;;) {
        task_run(GPIOA, TASK_B_PIN, TASK_B_WORK_US);
    }
}

void task_c(void *argument)
{
    (void)argument;
    for (;;) {
        task_run(GPIOA, TASK_C_PIN, TASK_C_WORK_US);

        /* Allow lower-priority tasks to execute */
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
}
