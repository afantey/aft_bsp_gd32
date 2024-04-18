/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-03-17     rgw             first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

extern sdk_uart_t uart0;
extern sdk_uart_t uart2;

__WEAK int gd32_uart_msp_init(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int gd32_uart_msp_deinit(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int32_t gd32_uart_write_dma(sdk_uart_t *uart, const uint8_t *data, uint32_t len)
{
    return -SDK_ERROR;
}

__WEAK int32_t gd32_uart_read_dma_config(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int32_t gd32_uart_get_dma_cnt(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int gd32_uart_update_state(sdk_uart_t *uart)
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
    else if(uart->instance == USART2)
    {
        /* enable USART clock */
        rcu_periph_clock_enable(RCU_USART2);
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

int32_t gd32_uart_write(sdk_uart_t *uart, const uint8_t *data, uint32_t len)
{
    for(int i = 0; i < len; i++)
    {
        gd32_uart_putc(uart, data[i]);
    }
    uart->txstate = UART_TX_COMPLETE;
    //callback
    uart->txstate = UART_TX_IDLE;
    return len;
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
    case SDK_CONTROL_UART_INT_IDLE_ENABLE:
        nvic_irq_enable(uart->irq, uart->irq_prio, 0);
        usart_interrupt_enable(uart->instance, USART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_DISABLE:
        usart_interrupt_disable(uart->instance, USART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_ENABLE_DMA:
        usart_dma_receive_config(uart->instance, USART_RECEIVE_DMA_ENABLE);
        usart_dma_transmit_config(uart->instance, USART_TRANSMIT_DMA_ENABLE);
        gd32_uart_read_dma_config(uart);
        uart->ops.write = gd32_uart_write_dma;
        break;
    case SDK_CONTROL_UART_DISABLE_DMA:
        usart_dma_receive_config(uart->instance, USART_RECEIVE_DMA_DISABLE);
        usart_dma_transmit_config(uart->instance, USART_TRANSMIT_DMA_DISABLE);
        uart->ops.write = gd32_uart_write;
        break;
    case SDK_CONTROL_UART_UPDATE_STATE:
        if(uart->ops.write == gd32_uart_write_dma)
        {
            gd32_uart_update_state(uart);
        }
        break;
    case SDK_CONTROL_UART_GET_DMA_CNT:
        *(uint32_t *)args = gd32_uart_get_dma_cnt(uart);
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
extern uint8_t uart2rxbuffer[256];
void USART2_IRQHandler(void)
{
    if ((usart_interrupt_flag_get(uart2.instance, USART_INT_FLAG_RBNE) != RESET) &&
        (usart_flag_get(uart2.instance, USART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(&uart2);
        usart_flag_clear(uart2.instance, USART_FLAG_RBNE);
    }
    if (usart_flag_get(uart2.instance, USART_FLAG_ORERR) != RESET)
    {
        usart_flag_clear(uart2.instance, USART_FLAG_ORERR);
    }

    if (usart_interrupt_flag_get(uart2.instance, USART_INT_FLAG_IDLE) != RESET)
    {
        /* clear IDLE flag */
        usart_data_receive(uart2.instance);

        /* number of data received */
        uint32_t rx_count = sizeof(uart2rxbuffer) - (dma_transfer_number_get(DMA0, DMA_CH1));

        if (rx_count < sizeof(uart2rxbuffer) / 2) // 半满中断前
        {
            for (int i = 0; i < rx_count; i++)
            {
                sdk_uart_rx_getc(&uart2, uart2rxbuffer[i]);
            }
        }
        else if (rx_count == sizeof(uart2rxbuffer) / 2)
        {
        }
        else if (rx_count > sizeof(uart2rxbuffer) / 2 && rx_count < sizeof(uart2rxbuffer))
        {
            for (int i = sizeof(uart2rxbuffer) / 2; i < rx_count; i++)
            {
                sdk_uart_rx_getc(&uart2, uart2rxbuffer[i]);
            }
        }
        else
        {
        }
        uart2.rxstate = UART_RX_COMPLETE;
        dma_channel_disable(DMA0, DMA_CH1); //disable时会产生一个满中断
        dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_HTF);
        dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_FTF);
        dma_transfer_number_config(DMA0, DMA_CH1, sizeof(uart2rxbuffer));
        dma_channel_enable(DMA0, DMA_CH1);
    }
}

sdk_uart_t uart0 = 
{
    .instance = USART0,
    .irq = USART0_IRQn,
    .irq_prio = 1,
    .ops.open = gd32_uart_open,
    .ops.close = gd32_uart_close,
    .ops.write = gd32_uart_write,
    .ops.putc = gd32_uart_putc,
    .ops.getc = gd32_uart_getc,
    .ops.control = gd32_uart_control,
    .rx_callback = NULL,
    .rx_idle_callback = NULL,
    .rx_rto_callback = NULL,
};

sdk_uart_t uart2 = 
{
    .instance = USART2,
    .irq = USART2_IRQn,
    .irq_prio = 1,
    .ops.open = gd32_uart_open,
    .ops.close = gd32_uart_close,
    .ops.write = gd32_uart_write,
    .ops.putc = gd32_uart_putc,
    .ops.getc = gd32_uart_getc,
    .ops.control = gd32_uart_control,
    .rx_callback = NULL,
    .rx_idle_callback = NULL,
    .rx_rto_callback = NULL,
};
