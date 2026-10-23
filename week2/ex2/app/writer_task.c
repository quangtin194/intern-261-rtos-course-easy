/* Week 2 / Ex2 — WRITER_COUNT identical writer tasks. Each prints an MSG_LEN
 * byte string with its own id + sequence number, every WRITER_PERIOD_MS.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"

/* TODO 1: one writer_task(void *argument) taking the writer index via the
 *         task parameter (argument), NOT WRITER_COUNT copy-pasted bodies.
 *         Format a MSG_LEN-byte string containing the writer id and a
 *         per-writer sequence number, then uart_guard_send() it, then
 *         vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS)). */
void writer_task(void *argument)
{
    uint32_t id = (uint32_t)(uintptr_t)argument;
    uint32_t seq = 0;


    for (;;) {
        /* TODO 2 */
        char msg[MSG_LEN];
        snprintf(msg, sizeof(msg), "ID: %01lu; Seq: %09lu\r\n",
                (unsigned long)id, (unsigned long)seq);
        uart_guard_send(msg, MSG_LEN);

        seq++;

        vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS));
    }
}
