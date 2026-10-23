/* Week 2 / Ex2 — race condition & mutex.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

void writer_task(void *argument);

void app_main(void)
{
    uart_guard_init();

    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */
    for (uint32_t i = 0; i < WRITER_COUNT; i++) {
        BaseType_t status = xTaskCreate(writer_task, "Write task", 128U, (void *)(uintptr_t)i, 1, NULL);
        configASSERT(status == pdPASS);
    }

    vTaskStartScheduler();

    for (;;) {
    }
}
