/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */
#include "sdk_board.h"
void sdk_hw_us_delay(uint32_t us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / SDK_SYSTICK_PER_SECOND);
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

void sdk_hw_interrupt_enable(void)
{
    __enable_irq();
}

void sdk_hw_interrupt_disable(void)
{
    __disable_irq();
}

#if 0 // It is usually implemented in sdk_board.c

static int __console_init = 0;

#define CONSOLE_INIT()             \
    do                             \
    {                              \
        if (!__console_init)       \
        {                          \
            sdk_hw_console_init(); \
            __console_init = 1;    \
        }                          \
    } while (0)

void sdk_hw_console_init(void)
{
    sdk_uart_open(&uart_console, 115200, 8, 'n', 1);
    sdk_uart_control(&uart_console, SDK_CONTROL_UART_DISABLE_INT, NULL);
}

void sdk_hw_console_output(const char *str)
{
    CONSOLE_INIT();
    sdk_uart_write(&uart_console, (uint8_t *)str, strlen(str));
}


uint32_t sdk_hw_get_systick(void)
{
    return rt_tick_get();
}

#endif
