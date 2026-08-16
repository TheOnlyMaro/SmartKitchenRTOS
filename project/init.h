#ifndef INIT_H
#define INIT_H

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"

#include "FreeRTOS.h"
#include "task.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* System clock configuration */
#define APP_SYS_CLOCK_CONFIG  (SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN)

/* Shared resources */
#define APP_UART_QUEUE_LENGTH                10U

/* Task configuration */
#define APP_LIGHTING_TASK_STACK_WORDS        256U
#define APP_LIGHTING_TASK_PRIORITY           (tskIDLE_PRIORITY + 2)
#define APP_OVEN_TASK_STACK_WORDS            320U
#define APP_OVEN_TASK_PRIORITY               (tskIDLE_PRIORITY + 2)
#define APP_UART_DRAIN_TASK_STACK_WORDS      256U
#define APP_UART_DRAIN_TASK_PRIORITY         (tskIDLE_PRIORITY + 1)

/* Lighting task behavior */
#define APP_LIGHTING_SAMPLE_PERIOD_MS        1000U
#define APP_SHARED_STATE_LOCK_TIMEOUT_MS     50U
#define APP_LIGHT_THRESHOLD_DARK             1000U

/* Lighting sensor wiring (change these for your board wiring) */
#define APP_LIGHT_ADC_GPIO_PERIPH            SYSCTL_PERIPH_GPIOE
#define APP_LIGHT_ADC_GPIO_BASE              GPIO_PORTE_BASE
#define APP_LIGHT_ADC_GPIO_PIN               GPIO_PIN_3

#define APP_LIGHT_ADC_PERIPH                 SYSCTL_PERIPH_ADC0
#define APP_LIGHT_ADC_BASE                   ADC0_BASE
#define APP_LIGHT_ADC_SEQUENCE               0U
#define APP_LIGHT_ADC_TRIGGER                ADC_TRIGGER_PROCESSOR
#define APP_LIGHT_ADC_PRIORITY               0U
#define APP_LIGHT_ADC_STEP_INDEX             0U
#define APP_LIGHT_ADC_STEP_CONFIG            (ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END)
#define APP_LIGHT_ADC_PHASE                  ADC_PHASE_0

/* Actuator selection */
#define APP_LIGHT_USE_BUILTIN_LED            1U

/* Optional external lighting output (used only when APP_LIGHT_USE_BUILTIN_LED is 0) */
#define APP_LIGHT_EXT_GPIO_PERIPH            SYSCTL_PERIPH_GPIOB
#define APP_LIGHT_EXT_GPIO_BASE              GPIO_PORTB_BASE
#define APP_LIGHT_EXT_GPIO_PIN               GPIO_PIN_0

/* Oven task behavior */
#define APP_OVEN_SAMPLE_PERIOD_MS            1000U
#define APP_OVEN_TEMP_THRESHOLD_C            34
#define APP_OVEN_TEMP_MIN_VALID_C            -10
#define APP_OVEN_TEMP_MAX_VALID_C            200

/* Oven actuator output wiring */
#define APP_OVEN_HEATER_GPIO_PERIPH          SYSCTL_PERIPH_GPIOF
#define APP_OVEN_HEATER_GPIO_BASE            GPIO_PORTF_BASE
#define APP_OVEN_HEATER_GPIO_PIN             GPIO_PIN_2

/* Oven sensor ADC wiring */
#define APP_OVEN_ADC_GPIO_PERIPH             SYSCTL_PERIPH_GPIOD
#define APP_OVEN_ADC_GPIO_BASE               GPIO_PORTD_BASE
#define APP_OVEN_ADC_GPIO_PIN                GPIO_PIN_0

#define APP_OVEN_ADC_PERIPH                  SYSCTL_PERIPH_ADC1
#define APP_OVEN_ADC_BASE                    ADC1_BASE
#define APP_OVEN_ADC_SEQUENCE                2U
#define APP_OVEN_ADC_TRIGGER                 ADC_TRIGGER_PROCESSOR
#define APP_OVEN_ADC_PRIORITY                2U
#define APP_OVEN_ADC_STEP_INDEX              0U
#define APP_OVEN_ADC_STEP_CONFIG             (ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END)
#define APP_OVEN_ADC_PHASE                   ADC_PHASE_180

/* LM35: temperature C = (adc * 330) / 4095, with 3.3V reference */
#define APP_OVEN_ADC_TO_C_SCALE_NUM          330
#define APP_OVEN_ADC_TO_C_SCALE_DEN          4095

void Init_SystemClock(void);
void Init_SharedResources(void);

// UART configuration
#define APP_UART_BAUD_RATE                   9600U
#define APP_UART_MODULE                        UART_MODULE_0

#endif /* INIT_H */