/* Week 3 / Ex2 -- software timer debounce.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "debounce.h"

void app_main(void)
{
    /* TODO: GPIO init for the button pin as EXTI, both edges
     *       (GPIO_MODE_IT_RISING_FALLING) -- you need every bounce edge, not
     *       just the first one. */
    // Đã cấu hình trong CubeMX và gen code ra
    debounce_init();

    vTaskStartScheduler();

    for (;;) {
    }
}
