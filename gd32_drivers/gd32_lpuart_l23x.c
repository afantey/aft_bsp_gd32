/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

#define DBG_TAG "bsp.lpuart"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

static int32_t gd32_lpuart_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    /* configure the CK_IRC16M as LPUART clock */
    rcu_lpuart_clock_config(RCU_LPUARTSRC_IRC16MDIV);

    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_LPUART);

    /* connect port to USART TX */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_0);
    /* connect port to USART RX */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_1);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_1);

    /* USART configure */
    lpuart_deinit();

    lpuart_invert_config(LPUART_SWAP_ENABLE);
    switch (data_bit)
    {
    case 9:
        lpuart_word_length_set(LPUART_WL_9BIT);
        break;
    case 8:
    default:
        lpuart_word_length_set(LPUART_WL_8BIT);
        break;
    }
    switch (stop_bit)
    {
    case 2:
        lpuart_stop_bit_set(LPUART_STB_2BIT);
        break;
    case 1:
    default:
        lpuart_stop_bit_set(LPUART_STB_1BIT);
        break;
    }
    switch (parity)
    {
    case 'e':
    case 'E':
        lpuart_parity_config(LPUART_PM_EVEN);
        break;
    case 'o':
    case 'O':
        lpuart_parity_config(LPUART_PM_ODD);
        break;
    case 'n':
    case 'N':
    default:
        lpuart_parity_config(LPUART_PM_NONE);
        break;
    }
    
    lpuart_baudrate_set(baudrate);
    lpuart_receive_config(LPUART_RECEIVE_ENABLE);
    lpuart_transmit_config(LPUART_TRANSMIT_ENABLE);

    nvic_irq_enable(LPUART_WKUP_IRQn, 0);
    exti_init(EXTI_28, EXTI_INTERRUPT, EXTI_TRIG_RISING);

    /* use start bit wakeup MCU */
    lpuart_wakeup_mode_config(LPUART_WUM_STARTB);
    lpuart_enable();
    /* ensure LPUART is enabled */
    while(RESET == lpuart_flag_get(LPUART_FLAG_REA)) {
    }
    /* check LPUART is not transmitting */
    while(SET == lpuart_flag_get(LPUART_FLAG_BSY)) {
    }
    lpuart_wakeup_enable();
    /* enable the WUIE interrupt */
    lpuart_interrupt_enable(LPUART_INT_WU);
    
    return SDK_OK;
}

static int32_t gd32_lpuart_close(sdk_uart_t *uart)
{
    lpuart_deinit();
    lpuart_disable();
    
    return SDK_OK;
}

static int32_t gd32_lpuart_putc(sdk_uart_t *uart, int32_t ch)
{
    while(RESET == lpuart_flag_get(LPUART_FLAG_TBE));
    lpuart_data_transmit((uint8_t) ch);
    while(RESET == lpuart_flag_get(LPUART_FLAG_TC));
    return ch;
}

static int32_t gd32_lpuart_getc(sdk_uart_t *uart)
{
    int ch;

    ch = -1;
    if (lpuart_flag_get(LPUART_FLAG_RBNE) != RESET)
        ch = lpuart_data_receive();
    return ch;
}

static int32_t gd32_lpuart_control(sdk_uart_t *uart, int32_t cmd, void *args)
{
    switch (cmd)
    {
    case SDK_CONTROL_UART_DISABLE_INT:
        /* disable rx irq */
        nvic_irq_disable(LPUART_IRQn);
        /* disable interrupt */
        lpuart_interrupt_disable(LPUART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_ENABLE_INT:
        /* enable rx irq */
        nvic_irq_enable(LPUART_IRQn, 0);
        /* enable interrupt */
        lpuart_interrupt_enable(LPUART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_ENABLE:
        /*wait IDLEF set and clear it*/
        while (RESET == lpuart_flag_get(LPUART_FLAG_IDLE))
        {
        }
        lpuart_flag_clear(LPUART_FLAG_IDLE);
        lpuart_interrupt_enable(LPUART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_DISABLE:
        lpuart_interrupt_disable(LPUART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_ENABLE_RX:
        lpuart_receive_config(LPUART_RECEIVE_ENABLE);
        break;
    case SDK_CONTROL_UART_DISABLE_RX:
        lpuart_receive_config(LPUART_RECEIVE_DISABLE);
        break;
    }

    return SDK_OK;
}

extern sdk_uart_t lpuart;

void LPUART_IRQHandler(void)
{
    if ((lpuart_interrupt_flag_get(LPUART_INT_FLAG_RBNE) != RESET) &&
            (lpuart_flag_get(LPUART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(&lpuart);
#if defined(SOC_SERIES_GD32L23x)
        lpuart_command_enable(LPUART_CMD_RXFCMD);
#else
        lpuart_flag_clear(LPUART_FLAG_RBNE);
#endif
    }
    if (lpuart_flag_get(LPUART_FLAG_ORERR) != RESET)
    {
        lpuart_flag_clear(LPUART_FLAG_ORERR);
    }
    if (RESET != lpuart_interrupt_flag_get(LPUART_INT_FLAG_IDLE))
    {
        lpuart_interrupt_flag_clear(LPUART_INT_FLAG_IDLE);
        if(lpuart.rx_idle_callback != NULL)
        {
            lpuart.rx_idle_callback();
        }
    }
}

__WEAK void lpuart_wakeup_callback(void)
{
    LOG_D("\n");
}
void LED_On(void);
void LPUART_WKUP_IRQHandler(void)
{
    if(SET == lpuart_interrupt_flag_get(LPUART_INT_FLAG_WU)) {
        lpuart_flag_clear(LPUART_FLAG_WU);
        lpuart_wakeup_callback();
        exti_flag_clear(EXTI_28);
    }
}

sdk_uart_t lpuart = 
{
    .ops.open = gd32_lpuart_open,
    .ops.close = gd32_lpuart_close,
    .ops.putc = gd32_lpuart_putc,
    .ops.getc = gd32_lpuart_getc,
    .ops.control = gd32_lpuart_control,
    .rx_callback = NULL,
    .rx_idle_callback = NULL,
    .rx_rto_callback = NULL,
};
