#include "oven_task.h"

#include <stdbool.h>
#include <stdint.h>

#include "init.h"
#include "shared_state.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "queue.h"
#include "semphr.h"

static void Oven_HwInit(void);
static int32_t Oven_ReadTemperatureC(uint32_t *pRawAdc);
static void Oven_SetHeater(bool on);
static void Oven_SendStatus(uint32_t rawAdc, int32_t tempC, bool heaterOn, bool overridden, bool fault);

void vOvenTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(APP_OVEN_SAMPLE_PERIOD_MS);
    int32_t tempC;
    uint32_t rawAdc = 0;
    bool overrideActive;
    bool overrideValue;
    bool heaterOn;
    bool fault;

    (void)pvParameters;

    Oven_HwInit();
    
    // Stagger the Oven task by 250ms relative to the Lighting task (500ms period)
    // to ensure they never sample the ADC concurrently.
    vTaskDelay(pdMS_TO_TICKS(250));
    
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (xSemaphoreTake(xSharedStateMutex, pdMS_TO_TICKS(APP_SHARED_STATE_LOCK_TIMEOUT_MS)) == pdTRUE)
        {
            overrideActive = g_SystemState.ovenOverrideActive;
            overrideValue = g_SystemState.ovenOverrideValue;
            xSemaphoreGive(xSharedStateMutex);
        }
        else
        {
            overrideActive = false;
            overrideValue = false;
        }

        tempC = Oven_ReadTemperatureC(&rawAdc);

        if ((tempC < APP_OVEN_TEMP_MIN_VALID_C) || (tempC > APP_OVEN_TEMP_MAX_VALID_C))
        {
            fault = true;
            heaterOn = false;
        }
        else
        {
            fault = false;
            if (overrideActive)
            {
                heaterOn = overrideValue;
            }
            else
            {
                heaterOn = (tempC < APP_OVEN_TEMP_THRESHOLD_C);
            }
        }

        Oven_SetHeater(heaterOn);
        Oven_SendStatus(rawAdc, tempC, heaterOn, overrideActive, fault);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void Oven_HwInit(void)
{
    SysCtlPeripheralEnable(APP_OVEN_ADC_GPIO_PERIPH);
    SysCtlPeripheralEnable(APP_OVEN_ADC_PERIPH);
    SysCtlPeripheralEnable(APP_OVEN_HEATER_GPIO_PERIPH);

    while(!SysCtlPeripheralReady(APP_OVEN_ADC_GPIO_PERIPH))
    {
    }
    while(!SysCtlPeripheralReady(APP_OVEN_ADC_PERIPH))
    {
    }
    while(!SysCtlPeripheralReady(APP_OVEN_HEATER_GPIO_PERIPH))
    {
    }

    GPIOPinTypeADC(APP_OVEN_ADC_GPIO_BASE, APP_OVEN_ADC_GPIO_PIN);
    GPIODirModeSet(APP_OVEN_ADC_GPIO_BASE, APP_OVEN_ADC_GPIO_PIN, GPIO_DIR_MODE_HW);

    ADCSequenceDisable(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE);

    ADCSequenceConfigure(APP_OVEN_ADC_BASE,
                         APP_OVEN_ADC_SEQUENCE,
                         APP_OVEN_ADC_TRIGGER,
                         APP_OVEN_ADC_PRIORITY);
    ADCSequenceStepConfigure(APP_OVEN_ADC_BASE,
                             APP_OVEN_ADC_SEQUENCE,
                             APP_OVEN_ADC_STEP_INDEX,
                             APP_OVEN_ADC_STEP_CONFIG);
    ADCPhaseDelaySet(APP_OVEN_ADC_BASE, APP_OVEN_ADC_PHASE);
    ADCSequenceEnable(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE);
    ADCIntClear(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE);

    GPIOPinTypeGPIOOutput(APP_OVEN_HEATER_GPIO_BASE, APP_OVEN_HEATER_GPIO_PIN);
    GPIOPinWrite(APP_OVEN_HEATER_GPIO_BASE, APP_OVEN_HEATER_GPIO_PIN, 0);
}

static int32_t Oven_ReadTemperatureC(uint32_t *pRawAdc)
{
    uint32_t sampleBuf[4];

    taskENTER_CRITICAL();
    ADCProcessorTrigger(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE);
    taskEXIT_CRITICAL();
    while (!ADCIntStatus(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE, false))
    {
        taskYIELD();
    }

    ADCIntClear(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE);
    ADCSequenceDataGet(APP_OVEN_ADC_BASE, APP_OVEN_ADC_SEQUENCE, sampleBuf);

    if (pRawAdc != NULL)
    {
        *pRawAdc = sampleBuf[0];
    }

    return (int32_t)((sampleBuf[0] * APP_OVEN_ADC_TO_C_SCALE_NUM) / APP_OVEN_ADC_TO_C_SCALE_DEN);
}

static void Oven_SetHeater(bool on)
{
    GPIOPinWrite(APP_OVEN_HEATER_GPIO_BASE,
                 APP_OVEN_HEATER_GPIO_PIN,
                 on ? APP_OVEN_HEATER_GPIO_PIN : 0);
}

static void Oven_SendStatus(uint32_t rawAdc, int32_t tempC, bool heaterOn, bool overridden, bool fault)
{
    UARTMessage_t msg;

    msg.source = fault ? SOURCE_SYSTEM : SOURCE_OVEN;
    msg.numericValue = ((uint32_t)((tempC < 0) ? 0 : tempC) << 16) | (rawAdc & 0xFFFF);
    msg.actuatorState = heaterOn;
    msg.isOverridden = overridden;
    msg.isFault = fault;

    (void)xQueueSend(xUARTQueue, &msg, 0);
}
