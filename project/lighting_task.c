/*
 * =====================================================================
 *  lighting_task.c
 *  Smart Kitchen System - Lighting Subsystem (Task 1)
 *  Member 1 deliverable
 * =====================================================================
 *  Responsibilities:
 *    - Sample ambient light level via LDR/ADC
 *    - Compare against a calibrated threshold to decide light ON/OFF
 *    - Respect manual override state (set by Override Task, Member 3)
 *    - Send formatted status updates to the central UART queue
 *
 *  Assumes: TM4C123 + TivaWare + FreeRTOS (matches board used in your
 *  RT Embedded Systems course). If this project actually targets a
 *  different MCU (e.g. STM32), the peripheral init calls below need
 *  to be swapped, but the task logic/structure stays the same.
 * =====================================================================
 */

#include "lighting_task.h"
#include "lighting_hal.h"
#include "init.h"
#include "shared_state.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static void Lighting_SendStatus(uint32_t adcValue, bool on, bool overridden);

/*
 * =====================================================================
 * vLightingTask
 *   Periodic task: sample -> decide -> actuate -> report.
 *   Priority: Normal (equal to Oven task, to exercise Round-Robin).
 * =====================================================================
 */
void vLightingTask(void *pvParameters)
{
    bool lightIsOn = false;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(APP_LIGHTING_SAMPLE_PERIOD_MS);
    uint32_t adcValue = 0;
    bool overrideActive;
    bool overrideOnValue;

    (void)pvParameters;

    LightingHal_Init();

    xLastWakeTime = xTaskGetTickCount();
    bool lightNewState;
    
    for(;;){

        // Check override state
        if (xSemaphoreTake(xSharedStateMutex, pdMS_TO_TICKS(APP_SHARED_STATE_LOCK_TIMEOUT_MS)) == pdTRUE)
        {
            overrideActive   = g_SystemState.lightingOverrideActive;
            overrideOnValue  = g_SystemState.lightingOverrideValue;
            xSemaphoreGive(xSharedStateMutex);
        }
        else
        {
            // Couldn't get the mutex in time - skip task cycle
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
            continue;
        }

        if(overrideActive){
            lightNewState = overrideOnValue;
            adcValue = 0; // ADC value is irrelevant when override is active
        } else {
            adcValue = LightingHal_ReadAmbient();
            lightNewState = (adcValue < APP_LIGHT_THRESHOLD_DARK);
        }

        if(lightNewState != lightIsOn){
            lightIsOn = lightNewState;
            LightingHal_SetOutput(lightIsOn);
            Lighting_SendStatus(adcValue, lightIsOn, overrideActive);
        }

        

        // Wait for next period (fixed-rate periodic task)
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

    }
    /*
    for (;;)
    {
        // --- 1. Sample ambient light sensor ---
        adcValue = LightingHal_ReadAmbient();

        // --- 2. Check shared override state (owned/updated by Override Task) ---
        // Shared state is protected by a mutex (owned by Member 3/4) since
        // both the Override Task and this task touch it.
        if (xSemaphoreTake(xSharedStateMutex, pdMS_TO_TICKS(APP_SHARED_STATE_LOCK_TIMEOUT_MS)) == pdTRUE)
        {
            overrideActive   = g_SystemState.lightingOverrideActive;
            overrideOnValue  = g_SystemState.lightingOverrideValue;
            xSemaphoreGive(xSharedStateMutex);
        }
        else
        {
            // Couldn't get the mutex in time - fail safe: treat as no override
            // this cycle rather than blocking the whole task indefinitely.
            overrideActive = false;
            overrideOnValue = false;
        }

        // --- 3. Decide light state ---
        if (overrideActive)
        {
            // Manual override takes precedence over sensor logic
            lightIsOn = overrideOnValue;
        }
        else
        {
            // Automatic threshold logic: darker than threshold -> lights ON
            lightIsOn = (adcValue < APP_LIGHT_THRESHOLD_DARK);
        }

        // --- 4. Actuate ---
        LightingHal_SetOutput(lightIsOn);

        // --- 5. Report status to UART logging task via queue ---
        Lighting_SendStatus(adcValue, lightIsOn, overrideActive);

        // --- 6. Wait for next period (fixed-rate periodic task) ---
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
    */
    
}

/*
 * =====================================================================
 * Lighting_SendStatus
 *   Builds a status message and pushes it onto the central UART queue.
 *   Non-blocking send with a short timeout: logging must never stall
 *   the control loop.
 * =====================================================================
 */
static void Lighting_SendStatus(uint32_t adcValue, bool on, bool overridden)
{
    UARTMessage_t msg;

    msg.source = SOURCE_LIGHTING;
    msg.numericValue = adcValue;
    msg.actuatorState = on;
    msg.isOverridden = overridden;
    msg.isFault = false;

    // non-blocking send (so we don't block the control cycle if the queue is full)
    xQueueSend(xUARTQueue, &msg, 0);
}