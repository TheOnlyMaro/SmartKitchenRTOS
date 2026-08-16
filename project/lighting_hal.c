#include "lighting_hal.h"

#include "init.h"
#include "led_builtin.h"

#include "FreeRTOS.h"
#include "task.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

void LightingHal_Init(void)
{
    SysCtlPeripheralEnable(APP_LIGHT_ADC_GPIO_PERIPH);
    SysCtlPeripheralEnable(APP_LIGHT_ADC_PERIPH);

    while(!SysCtlPeripheralReady(APP_LIGHT_ADC_GPIO_PERIPH))
    {
    }
    while(!SysCtlPeripheralReady(APP_LIGHT_ADC_PERIPH))
    {
    }

    GPIOPinTypeADC(APP_LIGHT_ADC_GPIO_BASE, APP_LIGHT_ADC_GPIO_PIN);
    GPIODirModeSet(APP_LIGHT_ADC_GPIO_BASE, APP_LIGHT_ADC_GPIO_PIN, GPIO_DIR_MODE_HW);

    ADCSequenceDisable(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE);

    ADCSequenceConfigure(APP_LIGHT_ADC_BASE,
                         APP_LIGHT_ADC_SEQUENCE,
                         APP_LIGHT_ADC_TRIGGER,
                         APP_LIGHT_ADC_PRIORITY);
    ADCSequenceStepConfigure(APP_LIGHT_ADC_BASE,
                             APP_LIGHT_ADC_SEQUENCE,
                             APP_LIGHT_ADC_STEP_INDEX,
                             APP_LIGHT_ADC_STEP_CONFIG);
    ADCPhaseDelaySet(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_PHASE);
    ADCSequenceEnable(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE);
    ADCIntClear(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE);

#if APP_LIGHT_USE_BUILTIN_LED
    LedBuiltin_Init();
#else
    SysCtlPeripheralEnable(APP_LIGHT_EXT_GPIO_PERIPH);
    while(!SysCtlPeripheralReady(APP_LIGHT_EXT_GPIO_PERIPH))
    {
    }
    GPIOPinTypeGPIOOutput(APP_LIGHT_EXT_GPIO_BASE, APP_LIGHT_EXT_GPIO_PIN);
    GPIOPinWrite(APP_LIGHT_EXT_GPIO_BASE, APP_LIGHT_EXT_GPIO_PIN, 0);
#endif
}

uint32_t LightingHal_ReadAmbient(void)
{
    uint32_t sampleBuffer[4];

    taskENTER_CRITICAL();
    ADCProcessorTrigger(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE);
    taskEXIT_CRITICAL();
    while (!ADCIntStatus(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE, false))
    {
        taskYIELD();
    }

    ADCIntClear(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE);
    ADCSequenceDataGet(APP_LIGHT_ADC_BASE, APP_LIGHT_ADC_SEQUENCE, sampleBuffer);

    return sampleBuffer[0];
}

void LightingHal_SetOutput(bool on)
{
#if APP_LIGHT_USE_BUILTIN_LED
    LedBuiltin_Set(on);
#else
    GPIOPinWrite(APP_LIGHT_EXT_GPIO_BASE,
                 APP_LIGHT_EXT_GPIO_PIN,
                 on ? APP_LIGHT_EXT_GPIO_PIN : 0);
#endif
}
