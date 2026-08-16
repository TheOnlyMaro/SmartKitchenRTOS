#include "uart_drain_task.h"

#include "shared_state.h"

#include "FreeRTOS.h"
#include "queue.h"

void vDummyUARTDrainTask(void *pvParameters)
{
    UARTMessage_t msg;

    (void)pvParameters;

    for (;;)
    {
        if (xQueueReceive(xUARTQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            volatile int breakpointHere = 0;
            (void)breakpointHere;
        }
    }
}
