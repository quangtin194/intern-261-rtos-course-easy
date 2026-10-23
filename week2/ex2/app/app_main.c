/* Week 2 / Ex2 — race condition & mutex.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

void writer_task(void *argument);

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* bật khối DWT */
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;            /* bắt đầu đếm */
}

void probe_task(void *a) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
    }
}

void app_main(void)
{
    dwt_init();
    uart_guard_init();
    size_t free = xPortGetFreeHeapSize();

    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */
    for (uint32_t i = 0; i < WRITER_COUNT; i++) {
        BaseType_t status = xTaskCreate(writer_task, "Write task", 128U, (void *)(uintptr_t)i, 1, NULL);
        configASSERT(status == pdPASS);
    }
    xTaskCreate(probe_task, "Probe", 128U, NULL, 4, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
}
