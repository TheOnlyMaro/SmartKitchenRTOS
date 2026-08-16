#include "led_builtin.h"

#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define LED_BUILTIN_PERIPH  SYSCTL_PERIPH_GPIOF
#define LED_BUILTIN_PORT    GPIO_PORTF_BASE
#define LED_BUILTIN_PIN     GPIO_PIN_1

void LedBuiltin_Init(void)
{
    SysCtlPeripheralEnable(LED_BUILTIN_PERIPH);
    GPIOPinTypeGPIOOutput(LED_BUILTIN_PORT, LED_BUILTIN_PIN);
    GPIOPinWrite(LED_BUILTIN_PORT, LED_BUILTIN_PIN, 0);
}

void LedBuiltin_Set(bool on)
{
    //control only the color of the built-in LED (PF1) without affecting other pins on the same port
    
    GPIOPinWrite(LED_BUILTIN_PORT, LED_BUILTIN_PIN, on ? LED_BUILTIN_PIN : 0);
}
