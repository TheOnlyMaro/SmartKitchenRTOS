#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART0_RX_BUFFER_SIZE 128U
#define UART0_TX_BUFFER_SIZE 128U


typedef enum
{
    UART_MODULE_0 = 0U,
    UART_MODULE_1,
    UART_MODULE_2,
    UART_MODULE_3,
    UART_MODULE_4,
    UART_MODULE_5,
    UART_MODULE_6,
    UART_MODULE_7
} UART_Module_t;

void UART_Init(UART_Module_t module);
void UART_config(UART_Module_t module, uint32_t baudRate);

void UART0_WriteChar(char c);
void UART0_WriteString(const char *str);
void UART0_ClearScreen(void);
char UART0_Read(void);
void UART0_Handler(void);

void UART0_ResetRxBuffer(void);
uint8_t UART0_IsEquationReady(void);
void UART0_GetEquation(char *dest, uint32_t maxLen);

#endif
