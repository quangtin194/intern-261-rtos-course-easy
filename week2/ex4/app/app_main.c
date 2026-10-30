/* Week 2 / Ex4 — event group & task notification.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "sync_tasks.h"

void app_main(void)
{
    sync_init();

    /* TODO 1: xTaskCreate sync_task first, store its handle into
     *         sync_task_handle (cast to void*) — producer_0 needs it when
     *         PRODUCER0_USE_NOTIFY is set. */
    xTaskCreate(sync_task, "sync", 256U, NULL, 5, (TaskHandle_t *)&sync_task_handle);

    /* TODO 2: xTaskCreate EVENT_BIT_COUNT instances of producer_task, each
     *         given its own index (0..EVENT_BIT_COUNT-1) as the task
     *         parameter. */
    for (uint32_t i = 0; i < EVENT_BIT_COUNT; i++) {
        xTaskCreate(producer_task, "producer", 128U, (void *)(uintptr_t)i, 1, NULL);
    }

    #if ANOTHER_TASK
    /* Dùng cho tình huống không dùng được notification
        Task này sẽ chờ notify để thực hiện toggle pin, nếu không thấy pin được toggle
        thì nghĩa là việc chờ notify không thành, do task gửi không gửi qua bên này */
        xTaskCreate(another_task, "another", 128U, NULL, 5, &another_task_handle);
    #endif
    //

    vTaskStartScheduler();

    for (;;) {
    }
}
