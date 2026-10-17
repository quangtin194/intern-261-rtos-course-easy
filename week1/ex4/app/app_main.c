/* Week 1 / Ex4 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "ram_report.h"

void ram_report_task(void *argument);   // .text
static void dyn_task(void *argument);   // .text
static void stat_task(void *argument);  // .text

TaskHandle_t dyn_task_Handle = NULL;            // 4 bytes .bss
TaskHandle_t ram_report_task_Handle = NULL;     // 4 bytes .bss
TaskHandle_t stat_task_Handle = NULL;           // 4 bytes .bss

/* TODO 1: static storage for stat_task — TCB and stack.
 *         The stack array type matters; it is not char[]. */
static StaticTask_t stat_task_TCB;                             // 76 bytes .bss
static StackType_t stat_task_Stack[STATIC_TASK_STACK_WORDS];   // 128*4 = 512 bytes .bss

static void shared_work(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO 2: identical trivial work in both tasks, so that any difference
         *         in high water mark comes from how they were created, not from
         *         what they do. */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void dyn_task (void *argument) {
    (void)argument;
    // volatile uint8_t arr[HOG_DEPTH_BYTES];
    // for (uint16_t i = 0; i < sizeof(arr); i++) {
    //     arr[i] = 0x01;
    // }
    shared_work(argument);
}
static void stat_task (void *argument) {
    (void)argument;
    shared_work(argument);
}
static void another_task (void *argument) {
    (void)argument;
    for (;;) {}
}

void app_main(void)
{
    /* TODO 3: dyn_task  via xTaskCreate,       stack DYN_TASK_STACK_WORDS
     *         stat_task via xTaskCreateStatic, stack STATIC_TASK_STACK_WORDS
     *         ram_report_task via xTaskCreate
     *
     * Check every return value. xTaskCreateStatic returns a handle, not a
     * BaseType_t — explain the difference in REPORT.md. */
    BaseType_t state_1 = xTaskCreate(dyn_task, "dyn task", (DYN_TASK_STACK_WORDS /* + 3 */), NULL, 1, &dyn_task_Handle);
    BaseType_t state_2 = xTaskCreate(ram_report_task, "ram report task", 200U, NULL, 2, &ram_report_task_Handle);
    xTaskCreate(another_task, "another task", DYN_TASK_STACK_WORDS, NULL, 1, NULL);

    stat_task_Handle = xTaskCreateStatic(stat_task, "stat task", STATIC_TASK_STACK_WORDS, NULL, 1, stat_task_Stack, &stat_task_TCB);
    
    if (state_1 != pdPASS) {
        UART_printSring("Create failed: dyn task \r\n");
        /*Handle Task creation failure*/
    }
        if (state_2 != pdPASS) {
        UART_printSring("Create failed: ram report task \r\n");
        /*Handle Task creation failure*/
    }
        if (stat_task_Handle == NULL) {
        UART_printSring("Create failed: stat task \r\n");
        /*Handle Task creation failure*/
    }

    /* TODO 4 (step 6 of the README, enable only for that run):
     *         declare a HOG_DEPTH_BYTES local array inside dyn_task's work
     *         function and write to every element, so the overflow is real and
     *         not optimised away. Think about what -O2 does to a dead array. */

    vTaskStartScheduler();

    /* Scheduler returned => not enough heap for the idle/timer task. */
    for (;;) {
    }
}
