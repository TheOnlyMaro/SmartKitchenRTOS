#ifndef UART_TASK_H
#define UART_TASK_H

#include "FreeRTOS.h"
#include "init.h"
#include "shared_state.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "UART.h"

void vUART_task(void* pvParameters);

#endif /* UART_TASK_H */