/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-03-17     rgw             first version
 */

#include "sdk_board.h"
#include "sdk_uart.h"

extern sdk_uart_t uart0;
extern sdk_uart_t uart1;
extern sdk_uart_t uart2;
extern sdk_uart_t uart6;

__WEAK int gd32_uart_msp_init(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

__WEAK int gd32_uart_msp_deinit(sdk_uart_t *uart)
{
    return -SDK_ERROR;
}

static int32_t gd32_uart_write_dma(sdk_uart_t *uart, const uint8_t *data, uint32_t len)
{
    if(len > uart->dma_config->tx_dma_buffer_size)
        return -1;
    
    memcpy(uart->dma_config->tx_dma_buffer, data, len);
    /* enable DMAx clock */
    rcu_periph_clock_enable(uart->dma_config->clock);

    dma_single_data_parameter_struct dma_init_struct;

    /* deinitialize DMAx channelx(USART TX) */
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (uint32_t)uart->dma_config->tx_dma_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = len;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(uart->instance);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel);
    dma_channel_subperipheral_select(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel, uart->dma_config->tx_dma_channel_subperipheral);
    /* enable DMAx channel7 transfer complete interrupt */
    // dma_interrupt_enable(DMAx, DMA_CH3, DMA_CHXCTL_FTFIE);
    /* enable DMAx channel7 */
    dma_channel_enable(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel);


    // nvic_irq_enable(DMAx_Channel2_IRQn, 0, 1);
    return SDK_OK;
}

static int32_t gd32_uart_rx_dma_config(sdk_uart_t *uart)
{
    dma_single_data_parameter_struct dma_init_struct;
    
    rcu_periph_clock_enable(uart->dma_config->clock);

    nvic_irq_enable(uart->dma_config->rx_dma_irq, uart->dma_config->rx_dma_irq_prio, 1);
    
    dma_deinit(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uint32_t)uart->dma_config->rx_dma_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = uart->dma_config->rx_dma_buffer_size;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(uart->instance);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, &dma_init_struct);
    
    /* configure DMA mode */
    // dma_circulation_disable(DMAx, DMA_CHx);
    dma_circulation_enable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);
    dma_channel_subperipheral_select(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, uart->dma_config->rx_dma_channel_subperipheral);

    dma_interrupt_enable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_HTF);
    dma_interrupt_enable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_FTF);

    /* enable DMAx channel2 */
    dma_channel_enable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);
    return SDK_OK;
}

static int32_t gd32_uart_get_dma_cnt(sdk_uart_t *uart)
{
    return dma_transfer_number_get(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);
}

static int gd32_uart_update_state(sdk_uart_t *uart)
{
    if (dma_flag_get(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel, DMA_FLAG_FTF) == SET)
    {
        dma_flag_clear(uart->dma_config->dma_instance, uart->dma_config->tx_dma_channel, DMA_FLAG_FTF);
        uart->txstate = UART_TX_COMPLETE;
        // do something
        uart->txstate = UART_TX_IDLE;
    }
    return SDK_OK;
}

static int32_t gd32_uart_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    // msp init
    if (gd32_uart_msp_init(uart) != SDK_OK)
    {
        return -SDK_ERROR;
    }

    /* enable USART clock */
    rcu_periph_clock_enable(uart->clock);

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
    {
        char *flag = (char *)args;
        if(args == NULL || (strchr(flag, 'r') != NULL))
        {
            /* enable rx irq */
            nvic_irq_enable(uart->irq, uart->irq_prio, 0);
            /* enable interrupt */
            usart_interrupt_enable(uart->instance, USART_INT_RBNE);
            break;
        }
    }
    case SDK_CONTROL_UART_INT_IDLE_ENABLE:
        nvic_irq_enable(uart->irq, uart->irq_prio, 0);
        usart_interrupt_enable(uart->instance, USART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_INT_IDLE_DISABLE:
        usart_interrupt_disable(uart->instance, USART_INT_IDLE);
        break;
    case SDK_CONTROL_UART_ENABLE_DMA:
    {
        if(args == NULL)
        {
            usart_dma_receive_config(uart->instance, USART_RECEIVE_DMA_ENABLE);
            gd32_uart_rx_dma_config(uart);
            usart_dma_transmit_config(uart->instance, USART_TRANSMIT_DMA_ENABLE);
            uart->ops.write = gd32_uart_write_dma;
        }
        else 
        {
            char *flag = (char *)args;
            if (strchr(flag, 'r') != NULL)
            {
                usart_dma_receive_config(uart->instance, USART_RECEIVE_DMA_ENABLE);
                gd32_uart_rx_dma_config(uart);
            }
            if (strchr(flag, 'w') != NULL)
            {
                usart_dma_transmit_config(uart->instance, USART_TRANSMIT_DMA_ENABLE);
                uart->ops.write = gd32_uart_write_dma;
            }
        }
        break;
    }
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

void _uart_isr(sdk_uart_t *uart)
{
    if ((usart_interrupt_flag_get(uart->instance, USART_INT_FLAG_RBNE) != RESET) &&
        (usart_flag_get(uart->instance, USART_FLAG_RBNE) != RESET))
    {
        sdk_uart_rx_isr(uart);
        usart_flag_clear(uart->instance, USART_FLAG_RBNE);
    }
    if (usart_flag_get(uart->instance, USART_FLAG_ORERR) != RESET)
    {
        usart_flag_clear(uart->instance, USART_FLAG_ORERR);
    }

    if (usart_interrupt_flag_get(uart->instance, USART_INT_FLAG_IDLE) != RESET)
    {
        /* clear IDLE flag */
        usart_data_receive(uart->instance);

        /* number of data received */
        uint32_t rx_count = uart->dma_config->rx_dma_buffer_size - (dma_transfer_number_get(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel));

        if (rx_count < uart->dma_config->rx_dma_buffer_size / 2) // 半满中断前
        {
            for (int i = 0; i < rx_count; i++)
            {
                sdk_uart_rx_getc(uart, uart->dma_config->rx_dma_buffer[i]);
            }
        }
        else if (rx_count == uart->dma_config->rx_dma_buffer_size / 2)
        {
        }
        else if (rx_count > uart->dma_config->rx_dma_buffer_size / 2 && rx_count < uart->dma_config->rx_dma_buffer_size)
        {
            for (int i = uart->dma_config->rx_dma_buffer_size / 2; i < rx_count; i++)
            {
                sdk_uart_rx_getc(uart, uart->dma_config->rx_dma_buffer[i]);
            }
        }
        else
        {
        }
        uart->rxstate = UART_RX_COMPLETE;
        dma_channel_disable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel); //disable时会产生一个满中断
        dma_flag_clear(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_FLAG_HTF);
        dma_flag_clear(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_FLAG_FTF);
        dma_transfer_number_config(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, uart->dma_config->rx_dma_buffer_size);
        dma_channel_enable(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);
    }
}

static void _dma_channel_isr(sdk_uart_t *uart)
{
    uint32_t dma_cnt = dma_transfer_number_get(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel);

    // 半满中断
    if (dma_interrupt_flag_get(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_FLAG_HTF))
    {
        dma_interrupt_flag_clear(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_FLAG_HTF);
        for (int i = 0; i < uart->dma_config->rx_dma_buffer_size - dma_cnt; i++)
        {
            sdk_uart_rx_getc(uart, uart->dma_config->rx_dma_buffer[i]);
        }
    }
    // 满中断
    if (dma_interrupt_flag_get(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(uart->dma_config->dma_instance, uart->dma_config->rx_dma_channel, DMA_INT_FLAG_FTF);
        if(uart->rxstate == UART_RX_COMPLETE)
        {
            uart->rxstate = UART_RX_IDLE;
        }
        else
        {
            for (int i = uart->dma_config->rx_dma_buffer_size / 2; i < uart->dma_config->rx_dma_buffer_size; i++)
            {
                sdk_uart_rx_getc(uart, uart->dma_config->rx_dma_buffer[i]);
            }
        }
    }
}

void USART1_IRQHandler(void)
{
    _uart_isr(&uart1);
}

void USART2_IRQHandler(void)
{
   _uart_isr(&uart2);
}

void UART6_IRQHandler(void)
{
   _uart_isr(&uart6);
}

void DMA0_Channel1_IRQHandler(void)
{
   _dma_channel_isr(&uart2);
}

void DMA0_Channel3_IRQHandler(void)
{
    _dma_channel_isr(&uart6);
}

void DMA0_Channel5_IRQHandler(void)
{
    _dma_channel_isr(&uart1);
}

#define UART1_TX_DMA_BUFFER_SIZE 512
#define UART1_RX_DMA_BUFFER_SIZE 256
static uint8_t uart1txbuffer[UART1_TX_DMA_BUFFER_SIZE] = {0};
static uint8_t uart1rxbuffer[UART1_RX_DMA_BUFFER_SIZE] = {0};

static struct sdk_uart_dma_config uart1_dma_config = 
{
    .dma_instance = DMA0,
    .clock = RCU_DMA0,

    .tx_dma_channel = DMA_CH6,
    .tx_dma_channel_subperipheral = DMA_SUBPERI4,
    .tx_dma_buffer = uart1txbuffer,
    .tx_dma_buffer_size = sizeof(uart1txbuffer),

    .rx_dma_channel = DMA_CH5,
    .rx_dma_channel_subperipheral = DMA_SUBPERI4,
    .rx_dma_buffer = uart1rxbuffer,
    .rx_dma_buffer_size = sizeof(uart1rxbuffer),
    .rx_dma_irq = DMA0_Channel5_IRQn,
    .rx_dma_irq_prio = 0,
};

#define UART2_TX_DMA_BUFFER_SIZE 256
static uint8_t uart2txbuffer[UART2_TX_DMA_BUFFER_SIZE] = {0};
#if 0
#define UART2_RX_DMA_BUFFER_SIZE 256
static uint8_t uart2rxbuffer[UART2_RX_DMA_BUFFER_SIZE] = {0};
#endif

static struct sdk_uart_dma_config uart2_dma_config = 
{
    .dma_instance = DMA0,
    .clock = RCU_DMA0,

    .tx_dma_channel = DMA_CH4,
    .tx_dma_channel_subperipheral = DMA_SUBPERI7,
    .tx_dma_buffer = uart2txbuffer,
    .tx_dma_buffer_size = sizeof(uart2txbuffer),

#if 0
    .rx_dma_channel = DMA_CH1,
    .rx_dma_channel_subperipheral = DMA_SUBPERI4,
    .rx_dma_buffer = uart2rxbuffer,
    .rx_dma_buffer_size = sizeof(uart2rxbuffer),
    .rx_dma_irq = DMA0_Channel1_IRQn,
    .rx_dma_irq_prio = 0,
#endif
};

// #define UART6_TX_DMA_BUFFER_SIZE 512
// static uint8_t uart6txbuffer[UART6_TX_DMA_BUFFER_SIZE] = {0};
#define UART6_RX_DMA_BUFFER_SIZE 512
static uint8_t uart6rxbuffer[UART6_RX_DMA_BUFFER_SIZE] = {0};

static struct sdk_uart_dma_config uart6_dma_config = 
{
    .dma_instance = DMA0,
    .clock = RCU_DMA0,
#if 0
    .tx_dma_channel = DMA_CH4,
    .tx_dma_channel_subperipheral = DMA_SUBPERI7,
    .tx_dma_buffer = uart2txbuffer,
    .tx_dma_buffer_size = sizeof(uart2txbuffer),
#endif
    .rx_dma_channel = DMA_CH3,
    .rx_dma_channel_subperipheral = DMA_SUBPERI5,
    .rx_dma_buffer = uart6rxbuffer,
    .rx_dma_buffer_size = sizeof(uart6rxbuffer),
    .rx_dma_irq = DMA0_Channel3_IRQn,
    .rx_dma_irq_prio = 0,
};


sdk_uart_t uart0 = {
    .instance = USART0,
    .clock = RCU_USART0,
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

sdk_uart_t uart1 = {
    .instance = USART1,
    .clock = RCU_USART1,
    .irq = USART1_IRQn,
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
    .dma_config = &uart1_dma_config,
};

sdk_uart_t uart2 = {
    .instance = USART2,
    .clock = RCU_USART2,
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
    .dma_config = &uart2_dma_config,
};

sdk_uart_t uart6 = {
    .instance = UART6,
    .clock = RCU_UART6,
    .irq = UART6_IRQn,
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
    .dma_config = &uart6_dma_config,
};
