/* Week 2 / Ex3 — priority inversion.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

void app_main(void)
{
    resource_init();
    BaseType_t status;

    /* TODO: xTaskCreate task_low, task_mid, task_high with three DISTINCT
     *       priorities, low < mid < high. Justify the exact numbers in
     *       REPORT.md — they must leave room for configMAX_PRIORITIES and not
     *       collide with any other task in the system. */

    status = xTaskCreate(task_low, "low high", 256U, NULL, 1, NULL);
    configASSERT(status == pdPASS);
    status = xTaskCreate(task_mid, "mid high", 256U, NULL, 2, NULL);
    configASSERT(status == pdPASS);
    status = xTaskCreate(task_high, "task high", 256U, NULL, 3, NULL);
    configASSERT(status == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}
