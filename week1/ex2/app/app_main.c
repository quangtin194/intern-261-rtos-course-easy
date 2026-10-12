/* Week 1 / Ex2 — priorities, preemption, starvation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"


void app_main(void)
{
    dwt_init();

    /* TODO: create the three tasks below with the priorities from params.h.
     *       Keep the stacks as small as you can still justify — ex4 will make
     *       you prove the number. */
    BaseType_t status_A = xTaskCreate(task_a, "Task A", 128U, NULL, TASK_A_PRIO, NULL);
    BaseType_t status_B = xTaskCreate(task_b, "Task B", 128U, NULL, TASK_A_PRIO, NULL);
    BaseType_t status_C = xTaskCreate(task_c, "Task C", 128U, NULL, TASK_A_PRIO, NULL);

    if (status_A != pdPASS) {
        for (;;) { /*Handle Task A creation failure*/ }
    }
    if (status_B != pdPASS) {
        for (;;) { /*Handle Task B creation failure*/ }
    }
    if (status_C != pdPASS) {
        for (;;) { /*Handle Task C creation failure*/ }

    }

    vTaskStartScheduler();

    for (;;) {
    }
}
