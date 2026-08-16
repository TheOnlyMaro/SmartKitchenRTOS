
#include "UART.h"

#include "stdint.h"

#include "tm4c123gh6pm_.h"



#define UART_SYSCLK_HZ 16000000UL

static volatile char g_uart0RxBuffer[UART0_RX_BUFFER_SIZE];
static volatile uint32_t g_uart0RxIndex = 0U;
static volatile uint8_t g_uart0EquationReady = 0U;

static void UART0_SetRxInterruptEnabled(uint8_t enable)
{
    if (enable != 0U)
    {
        UART0_ICR_R = (1U << 4);  /* Clear RX interrupt */
        UART0_IM_R |= (1U << 4);   /* Unmask RX interrupt */
        NVIC_EN0_R |= (1U << 5);   /* Enable IRQ 5 (UART0) */
    }
    else
    {
        UART0_IM_R &= ~(1U << 4);  /* Mask RX interrupt */
        NVIC_EN0_R &= ~(1U << 5);  /* Disable IRQ 5 (UART0) */
    }
}

static void UART0_EnableInterrupt(void)
{
    UART0_SetRxInterruptEnabled(1U);
}

void UART_Init(UART_Module_t module)
{
    if (module != UART_MODULE_0)
    {
        return;
    }

    SYSCTL_RCGCUART_R |= (1U << 0);
    SYSCTL_RCGCGPIO_R |= (1U << 0);

    while ((SYSCTL_PRUART_R & (1U << 0)) == 0U)
    {
    }

    while ((SYSCTL_PRGPIO_R & (1U << 0)) == 0U)
    {
    }

    UART0_CTL_R &= ~(1U << 0);

    GPIO_PORTA_AFSEL_R |= (1U << 0) | (1U << 1);
    GPIO_PORTA_DEN_R |= (1U << 0) | (1U << 1);
    GPIO_PORTA_AMSEL_R &= ~((1U << 0) | (1U << 1));
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~0x000000FFU) | 0x00000011U;
}

void UART_config(UART_Module_t module, uint32_t baudRate)
{
    uint32_t baud16;
    uint32_t remainder;
    uint32_t ibrd;
    uint32_t fbrd;

    if ((module != UART_MODULE_0) || (baudRate == 0U))
    {
        return;
    }

    UART0_CTL_R &= ~(1U << 0);

    baud16 = 16U * baudRate;
    ibrd = UART_SYSCLK_HZ / baud16;
    remainder = UART_SYSCLK_HZ % baud16;
    fbrd = ((remainder * 64U) + (baud16 / 2U)) / baud16;

    UART0_IBRD_R = ibrd;
    UART0_FBRD_R = fbrd;
    UART0_LCRH_R = (0x3U << 5); /* 8-bit, no parity, 1 stop bit, FIFO disabled */
    UART0_CC_R = 0x0U;

    UART0_CTL_R = (1U << 9) | (1U << 8) | (1U << 0);


}

void UART0_WriteChar(char c)
{
    while ((UART0_FR_R & (1U << 5)) != 0U)
    {
    }

    UART0_DR_R = (uint32_t)c;
}

void UART0_WriteString(const char *str)
{
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        UART0_WriteChar(*str);

        if (*str == '\n')
        {
            UART0_WriteChar('\r');
        }

        str++;
    }
}

void UART0_ClearScreen(void)
{
    UART0_WriteString("\033[2J\033[H");
}

char UART0_Read(void)
{
    while ((UART0_FR_R & (1U << 4)) != 0U)
    {
    }

    return (char)(UART0_DR_R & 0xFFU);
}

void UART0_ResetRxBuffer(void)
{
    UART0_SetRxInterruptEnabled(0U);
    g_uart0RxIndex = 0U;
    g_uart0EquationReady = 0U;
    g_uart0RxBuffer[0] = '\0';
    UART0_SetRxInterruptEnabled(1U);
}

uint8_t UART0_IsEquationReady(void)
{
    return g_uart0EquationReady;
}

void UART0_GetEquation(char *dest, uint32_t maxLen)
{
    uint32_t i;

    if ((dest == 0) || (maxLen == 0U))
    {
        return;
    }

    UART0_SetRxInterruptEnabled(0U);

    for (i = 0U; i < (maxLen - 1U); i++)
    {
        dest[i] = (char)g_uart0RxBuffer[i];
        if (dest[i] == '\0')
        {
            break;
        }
    }

    if (i == (maxLen - 1U))
    {
        dest[maxLen - 1U] = '\0';
    }

    g_uart0EquationReady = 0U;
    g_uart0RxIndex = 0U;
    g_uart0RxBuffer[0] = '\0';

    UART0_SetRxInterruptEnabled(1U);
}

void UART0_Handler(void)
{
    char rx;

    if ((UART0_MIS_R & (1U << 4)) == 0U)
    {
        return;
    }

    rx = (char)(UART0_DR_R & 0xFFU);
    UART0_ICR_R = (1U << 4);

    if ((rx == 'c') || (rx == 'C'))
    {
        g_uart0RxIndex = 0U;
        g_uart0EquationReady = 0U;
        g_uart0RxBuffer[0] = '\0';
        UART0_WriteString("\r\n[cleared]\r\n> ");
        return;
    }

    if (g_uart0EquationReady != 0U)
    {
        return;
    }

    UART0_WriteChar(rx);

    if ((g_uart0RxIndex + 1U) < UART0_RX_BUFFER_SIZE)
    {
        g_uart0RxBuffer[g_uart0RxIndex++] = rx;
        g_uart0RxBuffer[g_uart0RxIndex] = '\0';

        if (rx == '=')
        {
            g_uart0EquationReady = 1U;
            
        }
    }
    else
    {
        g_uart0RxIndex = 0U;
        g_uart0RxBuffer[0] = '\0';
        UART0_WriteString("\r\n[input too long, start again]\r\n> ");
    }
}
