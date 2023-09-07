/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */
#include "sdk_board.h"
#include "SEGGER_RTT.h"

void sdk_hw_console_output(const char *str)
{
    SEGGER_RTT_WriteString(0, str);
}

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

extern volatile uint32_t systicks;
uint32_t sdk_hw_get_systick(void)
{
    return systicks;
}
