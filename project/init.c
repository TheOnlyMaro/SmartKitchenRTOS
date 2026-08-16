#include "init.h"
#include "shared_state.h"
#include "queue.h"
#include "semphr.h"

void Init_SystemClock(void)
{
    SysCtlClockSet(APP_SYS_CLOCK_CONFIG);
}

void Init_SharedResources(void)
{
    xUARTQueue = xQueueCreate(APP_UART_QUEUE_LENGTH, sizeof(UARTMessage_t));
    xSharedStateMutex = xSemaphoreCreateMutex();

    g_SystemState.lightingOverrideActive = false;
    g_SystemState.lightingOverrideValue  = false;
    g_SystemState.ovenOverrideActive     = false;
    g_SystemState.ovenOverrideValue      = false;
}

