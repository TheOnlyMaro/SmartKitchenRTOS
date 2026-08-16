#include "shared_state.h"

SystemState_t g_SystemState;
QueueHandle_t xUARTQueue;
SemaphoreHandle_t xSharedStateMutex;
