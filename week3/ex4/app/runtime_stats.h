#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

#define SUPERVISOR_BIT_STATS   (1u << 2)

/* DWT CYCCNT based time base for configGENERATE_RUN_TIME_STATS. Wired into
 * FreeRTOSConfig.h via portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() and
 * portGET_RUN_TIME_COUNTER_VALUE() -- see app/FreeRTOSConfig.deltas. */
void runtime_stats_timer_init(void);
uint32_t runtime_stats_timer_value(void);

/* Prints a CPU% table (vTaskGetRunTimeStats) to UART every STATS_WINDOW_MS.
 * Runs forever; create as its own low-priority task. */
void runtime_stats_task(void *argument);
void Uart_printf(const char *str);

#endif /* RUNTIME_STATS_H */
