/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-03-17     rgw             first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

extern sdk_uart_t uart0;

__WEAK int gd32_uart_msp_init(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int gd32_uart_msp_deinit(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

static int32_t gd32_uart_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    // msp init
    if (gd32_uart_msp_init(uart) != SDK_OK)
    {
        return -SDK_ERROR;
    }

    if(uart->instance == USART0)
    {
        /* enable USART clock */
        rcu_periph_clock_enable(RCU_USART0);
    }
    else
    {
        return -SDK_ERROR;
    }

    /* USART configure */
    usart_deinit((uint32_t)uart->instance);
    switch (data_bit)
    {
    case 9:
        usart_word_length_set(uart->instance, USART_WL_9BIT);
        break;
    case 8:
    default:
        usart_word_length_set(uart->instance, USART_WL_8BIT);
        break;
    }
    switch (stop_bit)
    {
    case 2:
        usart_stop_bit_set(uart->instance, USART_STB_2BIT);
        break;
    case 1:
    default:
        usart_stop_bit_set(uart->instance, USART_STB_1BIT);
        break;
    }
    switch (parity)
    {
    case 'e':
    case 'E':
        usart_parity_config(uart->instance, USART_PM_EVEN);
        break;
    case 'o':
    case 'O':
        usart_parity_config(uart->instance, USART_PM_ODD);
        break;
    case 'n':
    case 'N':
    default:
        usart_parity_config(uart->instance, USART_PM_NONE);
        break;
    }
    
    usart_baudrate_set(uart->instance, baudrate);
    usart_receive_config(uart->instance, USART_RECEIVE_ENABLE);
    usart_transmit_config(uart->instance, USART_TRANSMIT_ENABLE);

    usart_enable(uart->instance);

    return SDK_OK;
}

static int32_t gd32_uart_close(sdk_uart_t *uart)
{
    usart_disable(uart->instance);
    usart_deinit(uart->instance);
    // msp deinit
    if (gd32_uart_msp_deinit(uart) != SDK_OK)
    {
        return -SDK_ERROR;
    }

    return SDK_OK;
}

static int32_t gd32_uart_putc(sdk_uart_t *uart, int32_t ch)
{
    usart_data_transmit(uart->instance, (uint8_t)ch);
    while(RESET == usart_flag_get(uart->instance, USART_FLAG_TBE));
    return ch;
}

static int32_t gd32_uart_getc(sdk_uart_t *uart)
{
    int ch = -1;
    if (usart_flag_get(uart->instance, USART_FLAG_RBNE) != RESET)
        ch = usart_data_receive(uart->instance);
    return ch;
}

static int32_t gd32_uart_control(sdk_uart_t *uart, int32_t cmd, void *args)
{
    switch (cmd)
    {
    case SDK_CONTROL_UART_DISABLE_INT:
        /* disable rx irq */
        nvic_irq_disable(uart->irq);
        /* disable interrupt */
        usart_interrupt_disable(uart->instance, USART_INT_RBNE);
        break;
    case SDK_CONTROL_UART_ENABLE_INT:
        /* enable rx irq */
        nvic_irq_enable(uart->irq, uart->irq_prio, 0);
        /* enable interrupt */
        usart_interrupt_enable(uart->instance, USART_INT_RBNE);
        break;
    default:
        return -SDK_E_INVALID;
    }


    return SDK_OK;
}


void USART0_IRQHandler(void)
{
    if ((usart_interrupt_flag_get(uart0.instance, USART_INT_FLAG_RBNE) != RESET) &&
            (usart_flag_get(uart0.instance, USART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(&uart0);
        usart_flag_clear(uart0.instance, USART_FLAG_RBNE);
    }
    if (usart_flag_get(uart0.instance, USART_FLAG_ORERR) != RESET)
    {
        usart_flag_clear(uart0.instance, USART_FLAG_ORERR);
    }
}


sdk_uart_t uart0 = 
{
    .instance = USART0,
    .irq = USART0_IRQn,
    .irq_prio = 1,
    .ops.open = gd32_uart_open,
    .ops.close = gd32_uart_close,
    .ops.putc = gd32_uart_putc,
    .ops.getc = gd32_uart_getc,
    .ops.control = gd32_uart_control,
    .rx_callback = NULL,
    .rx_idle_callback = NULL,
    .rx_rto_callback = NULL,
};
