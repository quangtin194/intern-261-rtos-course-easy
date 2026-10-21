/* Week 2 / Ex1 — reads MPU6050 over I2C1 every SAMPLE_PERIOD_MS, pushes a
 * raw_sample_t into queue_raw.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

#define MPU_ADDR (0x68 << 1)
#define MPU_WHO_AM_I_REG 0x75
#define MPU_DAT_REG_START 0x3B
#define MPU_WHO_AM_I_VALUE 0x68
#define MPU_PWR_MGMT_1_REG 0x6B

void sensor_task(void *argument)
{
    (void)argument;
    uint32_t seq = 0;

    /* TODO 1: HAL_I2C init / verify MPU6050 WHO_AM_I. Do this once here, not
     *         inside the loop. */
    uint8_t who_am_i = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, 
                    MPU_ADDR, 
                    MPU_WHO_AM_I_REG, 
                    I2C_MEMADD_SIZE_8BIT, 
                    &who_am_i, 
                    1, 
                    HAL_MAX_DELAY) != HAL_OK || who_am_i != MPU_WHO_AM_I_VALUE)
    {
        vTaskSuspend(NULL);
    }

    uint8_t wakeup = 0;
    if (HAL_I2C_Mem_Write(&hi2c1,
                      MPU_ADDR,
                      MPU_PWR_MGMT_1_REG,
                      I2C_MEMADD_SIZE_8BIT,
                      &wakeup,
                      1,
                      100) != HAL_OK)
    {
        vTaskSuspend(NULL);
    }

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        raw_sample_t s;
        s.seq = seq++;
        /* TODO 2: HAL_I2C_Mem_Read the accel registers into s.ax/ay/az. */
        uint8_t mpu_buffer[6];

        if (HAL_I2C_Mem_Read(&hi2c1, 
                        MPU_ADDR, 
                        MPU_DAT_REG_START, 
                        I2C_MEMADD_SIZE_8BIT, 
                        mpu_buffer, 
                        sizeof(mpu_buffer), 
                        HAL_MAX_DELAY) == HAL_OK)
        {
            
            s.ax = (int16_t)(((uint16_t)(mpu_buffer[0]) << 8) | mpu_buffer[1]);
            s.ay = (int16_t)(((uint16_t)(mpu_buffer[2]) << 8) | mpu_buffer[3]);
            s.az = (int16_t)(((uint16_t)(mpu_buffer[4]) << 8) | mpu_buffer[5]);
        

        /* TODO 3: xQueueSend(queue_raw, &s, <timeout>) — pick and justify the
         *         timeout in PREDICTION.md question 3/4. What happens to this
         *         sample if the queue is full and the timeout expires? */
            if (xQueueSend(queue_raw, &s, portMAX_DELAY) != pdPASS) { /* Send failed */ }
        }
        else { /* I2C read failed */ }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}
