/* Week 2 / Ex2 — four ways to serialize access to USART1 from WRITER_COUNT
 * tasks. Only one UART_GUARD_MODE is compiled at a time.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

#if UART_GUARD_MODE == 1
static SemaphoreHandle_t s_uart_mutex;
#elif UART_GUARD_MODE == 3
static QueueHandle_t s_gatekeeper_queue;
#endif

void uart_guard_init(void)
{
#if UART_GUARD_MODE == 1
    /* TODO 1: xSemaphoreCreateMutex(). Check for NULL. */
    s_uart_mutex = xSemaphoreCreateMutex();
    configASSERT(s_uart_mutex != NULL);

#elif UART_GUARD_MODE == 3
    /* TODO 2: create s_gatekeeper_queue (length: pick and justify in
     *         REPORT.md), item size = MSG_LEN bytes.
     *         xTaskCreate(gatekeeper_task, ..., GATEKEEPER_PRIO). */
    s_gatekeeper_queue = xQueueCreate(WRITER_COUNT, MSG_LEN);
    configASSERT(s_gatekeeper_queue != NULL);

    BaseType_t status = xTaskCreate(gatekeeper_task, "Gate keeper", 200U, NULL, GATEKEEPER_PRIO, NULL);
    configASSERT(status == pdPASS);

#endif
}

// static void uart_raw_send(const char *buf, size_t len)
// {
//     for (size_t i = 0; i < len; i++) {
//         HAL_UART_Transmit(&huart1, (uint8_t *)&buf[i], 1U, HAL_MAX_DELAY);
//         taskYIELD();
//     }
// }

static void uart_raw_send(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        while (!(USART1->SR & USART_SR_TXE)) { }
        USART1->DR = (uint8_t)buf[i];
    }
    while (!(USART1->SR & USART_SR_TC)) { }
}

//=====
volatile lat_stat_t g_lat;
void lat_record(uint32_t t0_cycles)
{
    uint32_t d = DWT->CYCCNT - t0_cycles;
    g_lat.sum += d;
    g_lat.cnt++;
    if (d > g_lat.max) g_lat.max = d;
}

#if UART_GUARD_MODE == 3
static volatile uint32_t s_t0[WRITER_COUNT];
void lat_mark(uint32_t id, uint32_t t0) { s_t0[id] = t0; }
#endif
// =====

void uart_guard_send(const char *buf, size_t len)
{
#if UART_GUARD_MODE == 0
    /* The bug: no serialization at all. */
    /* TODO 3: HAL_UART_Transmit directly. */
    uart_raw_send(buf, len);

#elif UART_GUARD_MODE == 1
    /* TODO 4: xSemaphoreTake(s_uart_mutex, portMAX_DELAY), transmit, Give.
     *         Every return path must Give — no early return between Take and
     *         Give. */
    if (xSemaphoreTake(s_uart_mutex, portMAX_DELAY) == pdPASS) 
    {
        uart_raw_send(buf, len);
        xSemaphoreGive(s_uart_mutex);
    }

#elif UART_GUARD_MODE == 2
    /* TODO 5: taskENTER_CRITICAL(), transmit, taskEXIT_CRITICAL().
     *         Measure in REPORT.md how long the critical section holds IRQs
     *         off, as a function of MSG_LEN and UART_BAUD. */
    taskENTER_CRITICAL();
    uart_raw_send(buf, len);
    taskEXIT_CRITICAL();

#elif UART_GUARD_MODE == 3
    /* TODO 6: xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY).
     *         The writer never touches UART directly in this mode. */
    xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY);

#endif
}

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument)
{
    (void)argument;
    for (;;) {
        char msg[MSG_LEN];
        /* TODO 7: xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY), then
         *         HAL_UART_Transmit. This is the only task allowed to call
         *         HAL_UART_Transmit in this mode. */
        if (xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY) == pdPASS) {
            uart_raw_send(msg, MSG_LEN);
            uint32_t id = (uint32_t)(msg[4] - '0');
            if (id < WRITER_COUNT) lat_record(s_t0[id]);
        }
    }
}
#endif
