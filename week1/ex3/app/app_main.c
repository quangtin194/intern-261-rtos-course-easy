/* Week 1 / Ex3 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void periodic_task(void *argument);
void dwt_init(void);
// Đo CPU overhead
// volatile uint32_t count = 0;

// void vMeasureTask (void *argument) {
//     (void)argument;
//     TickType_t start = xTaskGetTickCount();
//     while (xTaskGetTickCount() - start < pdMS_TO_TICKS(1000)) {
//         count++;
//     }
//     vTaskDelete(NULL);
// }


void app_main(void)
{
    dwt_init();

    /* TODO: create periodic_task. Its priority must be the highest in the system
     *       — justify why in REPORT.md, and say what the measurement would look
     *       like if it were not. */
    xTaskCreate(periodic_task, "Periodic task", 128U, NULL, 1, NULL);
    // Create task do CPU overhead
    // xTaskCreate(vMeasureTask, "Measure task", 128U, NULL, 0, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
}
