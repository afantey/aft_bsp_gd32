/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

static int32_t gd32_uart1_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USART TX */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);
    /* connect port to USART RX */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    switch (data_bit)
    {
    case 9:
        usart_word_length_set(USART1, USART_WL_9BIT);
        break;
    case 8:
    default:
        usart_word_length_set(USART1, USART_WL_8BIT);
        break;
    }
    switch (stop_bit)
    {
    case 2:
        usart_stop_bit_set(USART1, USART_STB_2BIT);
        break;
    case 1:
    default:
        usart_stop_bit_set(USART1, USART_STB_1BIT);
        break;
    }
    switch (parity)
    {
    case 'e':
    case 'E':
        usart_parity_config(USART1, USART_PM_EVEN);
        break;
    case 'o':
    case 'O':
        usart_parity_config(USART1, USART_PM_ODD);
        break;
    case 'n':
    case 'N':
    default:
        usart_parity_config(USART1, USART_PM_NONE);
        break;
    }
    
    usart_baudrate_set(USART1, baudrate);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);

    usart_enable(USART1);
    
    return SDK_OK;
}

static int32_t gd32_uart1_close(sdk_uart_t *uart)
{
    usart_deinit(USART1);
    usart_disable(USART1);
    
    return SDK_OK;
}

static int32_t gd32_uart1_putc(sdk_uart_t *uart, int32_t ch)
{
    usart_data_transmit(USART1, (uint8_t) ch);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}

static int32_t gd32_uart1_getc(sdk_uart_t *uart)
{
    int ch;

    ch = -1;
    if (usart_flag_get(USART1, USART_FLAG_RBNE) != RESET)
        ch = usart_data_receive(USART1);
    return ch;
}

static int32_t gd32_uart1_control(sdk_uart_t *uart, int32_t cmd, void *args)
{
    switch (cmd)
    {
    case SDK_CONTROL_UART_DISABLE_INT:
        /* disable rx irq */
        nvic_irq_disable(USART1_IRQn);
        /* disable interrupt */
        usart_interrupt_disable(USART1, USART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_ENABLE_INT:
        /* enable rx irq */
        nvic_irq_enable(USART1_IRQn, 0);
        /* enable interrupt */
        usart_interrupt_enable(USART1, USART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_ENABLE_RX:
        usart_receive_config(USART1, USART_RECEIVE_ENABLE);
        break;
    case SDK_CONTROL_UART_DISABLE_RX:
        usart_receive_config(USART1, USART_RECEIVE_DISABLE);
        break;
    }

    return SDK_OK;
}

extern sdk_uart_t uart1;

void USART1_IRQHandler(void)
{
    if ((usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE) != RESET) &&
            (usart_flag_get(USART1, USART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(&uart1);
#if defined(SOC_SERIES_GD32L23x)
        usart_command_enable(USART1, USART_CMD_RXFCMD);
#else
        usart_flag_clear(USART1, USART_FLAG_RBNE);
#endif
    }
    if (usart_flag_get(USART1, USART_FLAG_ORERR) != RESET)
        usart_flag_clear(USART1, USART_FLAG_ORERR);
}

sdk_uart_t uart1 = 
{
    .ops.open = gd32_uart1_open,
    .ops.close = gd32_uart1_close,
    .ops.putc = gd32_uart1_putc,
    .ops.getc = gd32_uart1_getc,
    .ops.control = gd32_uart1_control,
};
