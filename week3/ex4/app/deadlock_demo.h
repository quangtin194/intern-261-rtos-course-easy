#ifndef DEADLOCK_DEMO_H
#define DEADLOCK_DEMO_H
#include "semphr.h"

void deadlock_demo_init(void);
void task_alpha(void *argument);
void task_beta(void *argument);

SemaphoreHandle_t deadlock_demo_mutex_a(void);
SemaphoreHandle_t deadlock_demo_mutex_b(void);

#endif /* DEADLOCK_DEMO_H */
