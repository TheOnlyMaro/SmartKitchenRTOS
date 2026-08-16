/*
 * =====================================================================
 *  main.c
 *  Minimal entry point for testing vLightingTask IN ISOLATION.
 *
 *  This creates just enough shared infrastructure (queue + mutex) so
 *  lighting_task.c compiles and runs standalone, before Oven/Override/
 *  UART tasks exist. Once teammates' code is ready, this gets merged
 *  into the real integrated main.c (Member 4's job).
 * =====================================================================
 */

#include "FreeRTOS.h"
#include "task.h"

#include "lighting_task.h"
#include "oven_task.h"
#include "init.h"
#include "uart_drain_task.h"
#include "UART.h"
#include "UART_task.h"


int main(void)
{
    Init_SystemClock();
    Init_SharedResources();
    UART_Init(APP_UART_MODULE);
    UART_config(APP_UART_MODULE, APP_UART_BAUD_RATE);
    UART0_WriteString("System startup\n");

    xTaskCreate(vLightingTask,
                "Lighting",
                APP_LIGHTING_TASK_STACK_WORDS,
                NULL,
                APP_LIGHTING_TASK_PRIORITY,
                NULL);

    xTaskCreate(vOvenTask,
                "Oven",
                APP_OVEN_TASK_STACK_WORDS,
                NULL,
                APP_OVEN_TASK_PRIORITY,
                NULL);

    xTaskCreate(vUART_task,
                "UART task",
                APP_UART_DRAIN_TASK_STACK_WORDS,
                NULL,
                APP_UART_DRAIN_TASK_PRIORITY,
                NULL);

		UART0_WriteString("starting scheduler\n");
    vTaskStartScheduler();

    for (;;)
    {
    }
}