/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

static int32_t gd32_uart0_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USART TX */
    gpio_af_set(GPIOC, GPIO_AF_7, GPIO_PIN_4);
    /* connect port to USART RX */
    gpio_af_set(GPIOC, GPIO_AF_7, GPIO_PIN_5);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_4);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_5);

    /* USART configure */
    usart_deinit(USART0);
    switch (data_bit)
    {
    case 9:
        usart_word_length_set(USART0, USART_WL_9BIT);
        break;
    case 8:
    default:
        usart_word_length_set(USART0, USART_WL_8BIT);
        break;
    }
    switch (stop_bit)
    {
    case 2:
        usart_stop_bit_set(USART0, USART_STB_2BIT);
        break;
    case 1:
    default:
        usart_stop_bit_set(USART0, USART_STB_1BIT);
        break;
    }
    switch (parity)
    {
    case 'e':
    case 'E':
        usart_parity_config(USART0, USART_PM_EVEN);
        break;
    case 'o':
    case 'O':
        usart_parity_config(USART0, USART_PM_ODD);
        break;
    case 'n':
    case 'N':
    default:
        usart_parity_config(USART0, USART_PM_NONE);
        break;
    }
    
    usart_baudrate_set(USART0, baudrate);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_enable(USART0);
    
    return SDK_OK;
}

static int32_t gd32_uart0_close(sdk_uart_t *uart)
{
    usart_disable(USART0);
    usart_deinit(USART0);
    
    return SDK_OK;
}

static int32_t gd32_uart0_putc(sdk_uart_t *uart, int32_t ch)
{
    usart_data_transmit(USART0, (uint8_t) ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return ch;
}

static int32_t gd32_uart0_getc(sdk_uart_t *uart)
{
    int ch;

    ch = -1;
    if (usart_flag_get(USART0, USART_FLAG_RBNE) != RESET)
        ch = usart_data_receive(USART0);
    return ch;
}

static int32_t gd32_uart0_control(sdk_uart_t *uart, int32_t cmd, void *args)
{
    switch (cmd)
    {
    case SDK_CONTROL_UART_DISABLE_INT:
        /* disable rx irq */
        nvic_irq_disable(USART0_IRQn);
        /* disable interrupt */
        usart_interrupt_disable(USART0, USART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_ENABLE_INT:
        /* enable rx irq */
        nvic_irq_enable(USART0_IRQn, 0);
        /* enable interrupt */
        usart_interrupt_enable(USART0, USART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_ENABLE:
        /*wait IDLEF set and clear it*/
        while (RESET == usart_flag_get(USART0, USART_FLAG_IDLE))
        {
        }
        usart_flag_clear(USART0, USART_FLAG_IDLE);
        usart_interrupt_enable(USART0, USART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_DISABLE:
        break;
    case SDK_CONTROL_UART_INT_RTO_ENABLE:
        /* enable the USART receive timeout and configure the time of timeout */
        usart_receiver_timeout_enable(USART0);
        usart_interrupt_enable(USART0, USART_INT_RT);
        usart_receiver_timeout_threshold_config(USART0, *(uint32_t *)args);
        break;
    case SDK_CONTROL_UART_INT_RTO_DISABLE:
        break;
    case SDK_CONTROL_UART_ENABLE_RX:
        usart_receive_config(USART0, USART_RECEIVE_ENABLE);
        break;
    case SDK_CONTROL_UART_DISABLE_RX:
        usart_receive_config(USART0, USART_RECEIVE_DISABLE);
        break;
    }

    return SDK_OK;
}

extern sdk_uart_t uart0;



void USART0_IRQHandler(void)
{
    if ((usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET) &&
            (usart_flag_get(USART0, USART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(&uart0);
#if defined(SOC_SERIES_GD32L23x)
        usart_command_enable(USART0, USART_CMD_RXFCMD);
#else
        usart_flag_clear(USART0, USART_FLAG_RBNE);
#endif
    }
    if (usart_flag_get(USART0, USART_FLAG_ORERR) != RESET)
    {
        usart_flag_clear(USART0, USART_FLAG_ORERR);
    }
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
        if(uart0.rx_idle_callback != NULL)
        {
            uart0.rx_idle_callback();
        }
    }
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT))
    {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RT);
        if(uart0.rx_rto_callback != NULL)
        {
            uart0.rx_rto_callback();
        }
    }
}

sdk_uart_t uart0 = 
{
    .ops.open = gd32_uart0_open,
    .ops.close = gd32_uart0_close,
    .ops.putc = gd32_uart0_putc,
    .ops.getc = gd32_uart0_getc,
    .ops.control = gd32_uart0_control,
    .rx_callback = NULL,
    .rx_idle_callback = NULL,
    .rx_rto_callback = NULL,
};
