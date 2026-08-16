#include "init.h"
#include "shared_state.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "UART.h"
#include "UART_task.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static void uint_to_str(uint32_t val, char *buf, size_t buf_size)
{
    char tmp[12];
    int i = 0;
    if (val == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while (val > 0 && i < 11)
        {
            tmp[i++] = '0' + (val % 10);
            val /= 10;
        }
    }
    
    size_t out_len = 0;
    while (i > 0 && out_len < buf_size - 2)
    {
        buf[out_len++] = tmp[--i];
    }
    buf[out_len++] = '\n';
    buf[out_len] = '\0';
}

void vUART_task(void *pvParameters)
{
    UARTMessage_t msg;

    (void)pvParameters;

    // Display startup message
    UART0_WriteString("\r\n==========================================\r\n");
    UART0_WriteString("Smart Kitchen System Log Task Online (Output Only)\r\n");
    UART0_WriteString("==========================================\r\n");

    for (;;)
    {
        // Block indefinitely until a message is received
        if (xQueueReceive(xUARTQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            // Process and log the received UART message
            UART0_WriteString("\r\n--- UART Log Message ---\r\n");
            UART0_WriteString("Source: ");
            switch (msg.source)
            {
                case SOURCE_LIGHTING:
                    UART0_WriteString("Lighting\r\n");
                    break;
                case SOURCE_OVEN:
                    UART0_WriteString("Oven\r\n");
                    break;
                case SOURCE_OVERRIDE:
                    UART0_WriteString("Override\r\n");
                    break;
                case SOURCE_SYSTEM:
                    UART0_WriteString("System\r\n");
                    break;
                default:
                    UART0_WriteString("Unknown\r\n");
                    break;
            }
            if (msg.source == SOURCE_OVEN)
            {
                uint32_t rawAdc = msg.numericValue & 0xFFFF;
                int32_t tempC = (int32_t)(msg.numericValue >> 16);
                char buffer[16];

                UART0_WriteString("Temp (C): ");
                uint_to_str((uint32_t)tempC, buffer, sizeof(buffer));
                UART0_WriteString(buffer);

                UART0_WriteString("Raw ADC: ");
                uint_to_str(rawAdc, buffer, sizeof(buffer));
                UART0_WriteString(buffer);
            }
            else
            {
                UART0_WriteString("Numeric Value: ");
                char buffer[16];
                uint_to_str(msg.numericValue, buffer, sizeof(buffer));
                UART0_WriteString(buffer);
            }

            UART0_WriteString("Actuator State: ");
            UART0_WriteString(msg.actuatorState ? "ON\r\n" : "OFF\r\n");
            UART0_WriteString("Is Overridden: ");
            UART0_WriteString(msg.isOverridden ? "Yes\r\n" : "No\r\n");
            UART0_WriteString("Is Fault: ");
            UART0_WriteString(msg.isFault ? "Yes\r\n" : "No\r\n");
            UART0_WriteString("-------------------------\r\n");
        }
    }
}
