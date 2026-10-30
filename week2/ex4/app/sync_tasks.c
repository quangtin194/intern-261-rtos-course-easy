/* Week 2 / Ex4 — event group sync across EVENT_BIT_COUNT producers, with
 * producer_0's signal optionally replaced by a task notification.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "sync_tasks.h"
#include <stm32f103xb.h>
#include <stdio.h>
#include "dwt.h"
#include "params.h"

#define ALL_BITS_MASK ((1U << EVENT_BIT_COUNT) - 1U)
#define PRODUCER1_BIT (1U << 1) 

// Đo time
static uint32_t producer_t0[EVENT_BIT_COUNT];  // t0 arr
static uint32_t producer_t1;                   // t1 nhom
static uint32_t spec_t1;                       // t1 rieng khi do tung producer
static uint32_t delta_t[EVENT_BIT_COUNT];
// Đo RAM
static size_t ram_bef;
static size_t ram_aft;
//
#if ANOTHER_TASK
TaskHandle_t another_task_handle = NULL;
#endif
// Random work time
static uint32_t s_rand_state = 0x2413505UL;
static uint32_t next_work_ms(void)
{
    uint32_t x = s_rand_state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    s_rand_state = x;

    return 2U + (x % 3U);   // random 2 - 4 ms
}

static EventGroupHandle_t s_events;
void *sync_task_handle;

void sync_init(void)
{
    /* TODO 1: xEventGroupCreate(). Check for NULL.
     *         Measure and record in REPORT.md the sizeof the structure this
     *         allocates (see event_groups.c in the kernel sources) to answer
     *         PREDICTION.md question 1. */
    dwt_init();

    void *warm = pvPortMalloc(1U);
    vPortFree(warm);

    ram_bef = xPortGetFreeHeapSize();
    s_events = xEventGroupCreate();
    ram_aft = xPortGetFreeHeapSize();
    configASSERT(s_events != NULL);
}

void producer_task(void *argument)
{
    uint32_t idx = (uint32_t)(uintptr_t)argument;

    for (;;) {
        /* TODO 2: random small period (reuse a simple xorshift like ex3's
         *         periodic task, or vTaskDelay with a bounded random jitter). */
     // Giúp producer hoàn thành công việc ở những thời điểm khác nhau
     uint32_t work_time = next_work_ms();
    dwt_busy_wait_us(work_time * 1000U);

#if PRODUCER0_USE_NOTIFY
        if (idx == 0) {
            /* TODO 3: xTaskNotifyGive((TaskHandle_t)sync_task_handle).
             *         Toggle a DWT-timestamped GPIO edge right before this
             *         call — this is your t0 for the wake-up latency
             *         measurement. */
            producer_t0[idx] = DWT->CYCCNT;
            xTaskNotifyGive((TaskHandle_t)sync_task_handle);
            // xTaskNotifyGive((TaskHandle_t)another_task_handle);
        } else
#endif
        {
            /* TODO 4: xEventGroupSetBits(s_events, 1u << idx).
             *         Same timestamp discipline as TODO 3 for producers that
             *         still use the event group. */
            #if SYNC_TASK_TIMEOUT
            if (idx == 2) dwt_busy_wait_us(100 * 1000U); // Report ques4, workload 100ms > SYNC_TIMEOUT_MS
            #endif

            producer_t0[idx] = DWT->CYCCNT;
            xEventGroupSetBits(s_events, 1U << idx);
        }
    }
}

void sync_task(void *argument)
{
    (void)argument;
    EventBits_t uxBits;

    for (;;) {
        BaseType_t sync_ok = pdFALSE;
        EventBits_t remain_mask = 0;

#if PRODUCER0_USE_NOTIFY
        /* TODO 5: ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYNC_TIMEOUT_MS)) for
         *         producer_0's contribution, THEN xEventGroupWaitBits for the
         *         remaining producers' bits (mask without bit 0), clear-on-exit
         *         = pdTRUE, wait-for-all = pdTRUE, same timeout.
         *         Toggle a GPIO edge the moment both are satisfied — your t1
         *         for the latency measurement. */
        uint32_t notify_count = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYNC_TIMEOUT_MS));
        spec_t1 = DWT->CYCCNT;    // Đo producer_0 riêng

        if (notify_count > 0) 
        {
            remain_mask = (EventBits_t)(ALL_BITS_MASK & ~1U); 
            uxBits = xEventGroupWaitBits(
                                s_events,
                                remain_mask,
                                pdTRUE,
                                pdTRUE,
                                pdMS_TO_TICKS(SYNC_TIMEOUT_MS)
            );
            if (((uxBits & remain_mask) == remain_mask)) 
            {
                sync_ok = pdTRUE;

            } else xEventGroupClearBits(s_events, remain_mask);
        }
#else
        /* TODO 6: xEventGroupWaitBits(s_events, ALL_BITS_MASK, pdTRUE, pdTRUE,
         *         pdMS_TO_TICKS(SYNC_TIMEOUT_MS)).
         *         Check the return value against ALL_BITS_MASK to tell
         *         "all bits arrived" apart from "timed out with some bits
         *         missing" — do not assume a non-zero return means success. */
        uxBits = xEventGroupWaitBits(
                                    s_events,
                                    ALL_BITS_MASK,
                                    pdTRUE,
                                    pdTRUE,
                                    pdMS_TO_TICKS(SYNC_TIMEOUT_MS)
        );
        if ((uxBits & ALL_BITS_MASK) == ALL_BITS_MASK) {
            sync_ok = pdTRUE;

        } else 
        {
            #if SYNC_TASK_TIMEOUT
            char buffer[64] = "Sync_task timeout\r\n";
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)sizeof(buffer), HAL_MAX_DELAY);
            snprintf(buffer, sizeof(buffer), "xEventGroupWaitBits: 0x%lx\r\n", (unsigned long)uxBits);
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)sizeof(buffer), HAL_MAX_DELAY);
            #endif

            xEventGroupClearBits(s_events, ALL_BITS_MASK);
            // vTaskSuspend(NULL);
        }
#endif

        /* TODO 7: print the sync instant and measured period to UART. */
        if (sync_ok == pdTRUE) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1);
            
            // Đo producer sync task
            producer_t1 = DWT->CYCCNT;
            #if PRODUCER0_USE_NOTIFY
            char buffer[64] = "Event Group 3 point 1 notification:\r\n";
            #else
            char buffer[64] = "Event Group 4 point:\r\n";
            #endif
            int len;
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)sizeof(buffer), HAL_MAX_DELAY);

            for (uint32_t i = 0; i < EVENT_BIT_COUNT; i++) {
                delta_t[i] = producer_t1 - producer_t0[i];
                uint32_t wake_time = delta_t[i] / 72; 

                len = snprintf(buffer, sizeof(buffer), "cycle: %lu, us: %lu\r\n", (unsigned long)delta_t[i], (unsigned long)wake_time);
                HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);
            }
            //

            // Đo riêng thời gian đánh thức producer
            #if PRODUCER0_USE_NOTIFY
            len = snprintf(buffer, sizeof(buffer), "Wake time pr0: %lu cycle\r\n", (unsigned long)(spec_t1 - producer_t0[0]));
            #else
            len = snprintf(buffer, sizeof(buffer), "Wake time pr3: %lu cycle\r\n", (unsigned long)(producer_t1 - producer_t0[3]));
            #endif
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);
            //

            // Đo RAM của Event Group
            len = snprintf(buffer, sizeof(buffer), "Sizeof Event Gr: %u bytes\r\n", (unsigned int)get_event_group_size());
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);

            len = snprintf(buffer, sizeof(buffer), "RAM used: %lu bytes\r\n", (unsigned long)(ram_bef - ram_aft));
            HAL_UART_Transmit(&huart1,(uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);
            //

            #if ANOTHER_TASK
            #else
            // vTaskSuspendAll();
            #endif
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 0);
        }
    }
}

#if ANOTHER_TASK
void another_task(void *argument) {
    (void)argument;

    for (;;) {
        uint32_t notify_count = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(portMAX_DELAY));
        if (notify_count > 0) HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

        vTaskDelay(2);
    }
}
#endif
