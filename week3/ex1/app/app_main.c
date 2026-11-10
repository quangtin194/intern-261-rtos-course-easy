/* Week 3 / Ex1 -- ISR -> task wakeup, NVIC priority.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "exti_button.h"

void app_main(void)
{
    exti_button_init();

    /* TODO: xTaskCreate ack_task. Keep its priority high enough that it is
     *       the thing that actually determines your measured latency, not
     *       scheduling jitter from something else. Justify the number in
     *       REPORT.md. */
    xTaskCreate(ack_task, "AckTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
}
