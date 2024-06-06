/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-06-04     rgw             first version
 */

#include "aft_sdk.h"
#include "sdk_can.h"
#include "sdk_board.h"

#define DBG_TAG "bsp.can"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

extern sdk_can_t can0;
__WEAK int gd32_can_msp_init(sdk_can_t *can)
{
    return -SDK_ERROR;
}

__WEAK int gd32_can_msp_deinit(sdk_can_t *can)
{
    return -SDK_ERROR;
}

static void nvic_config(sdk_can_t *can)
{
    nvic_irq_enable(can->irq, can->irq_prio, 0);
}

static void can_networking_init(sdk_can_t *can)
{
    can_parameter_struct            can_parameter;
    can_filter_parameter_struct     can_filter;
    
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    
    /* initialize CAN register */
    can_deinit(can->instance);
    
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_11TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_3TQ;
    /* baudrate 500kbps */
    can_parameter.prescaler = 8;
    can_init(can->instance, &can_parameter);

    /* initialize filter */
    /* CAN0 filter number */
    can_filter.filter_number = 0;

    /* initialize filter */    
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
    can_filter.filter_fifo_number = can->can_fifo_num;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
}

sdk_err_t gd32_can_open(sdk_can_t *can)
{
    // msp init
    if (gd32_can_msp_init(can) != SDK_OK)
    {
        return -SDK_ERROR;
    }

    // clock enable
    rcu_periph_clock_enable(can->clock);
    
    nvic_config(can);
    can_networking_init(can);
    /* enable CAN receive FIFO0 not empty interrupt */
    if(can->can_fifo_num == CAN_FIFO0)
        can_interrupt_enable(can->instance, CAN_INT_RFNE0);
    else if(can->can_fifo_num == CAN_FIFO1)
        can_interrupt_enable(can->instance, CAN_INT_RFNE1);
    else
        return -SDK_ERROR;

    return SDK_OK;
}

sdk_err_t gd32_can_close(sdk_can_t *can)
{
    can_deinit(can->instance);
    return SDK_OK;
}

int32_t gd32_can_read(sdk_can_t *can, sdk_can_msg_t *msg)
{
    can_receive_message_struct receive_message;
    can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &receive_message);
    can_message_receive(can->instance, can->can_fifo_num, &receive_message);
    
    if(CAN_FF_STANDARD == receive_message.rx_ff)
    {
        msg->canid = receive_message.rx_sfid;
        msg->ide = SDK_CAN_IDE_STANDARD;
    }
    else if(CAN_FF_EXTENDED == receive_message.rx_ff)
    {
        msg->canid = receive_message.rx_efid;
        msg->ide = SDK_CAN_IDE_EXTENDED;
    }

    if(CAN_FT_DATA == receive_message.rx_ft)
    {
        msg->rtr = SDK_CAN_RTR_DATA;
    }
    else if(CAN_FT_REMOTE == receive_message.rx_ft)
    {
        msg->rtr = SDK_CAN_RTR_REMOTE;
    }

    msg->dlc = receive_message.rx_dlen;
    if(receive_message.rx_dlen > 8)
    {
        return 0;
    }

    for (int i = 0; i < receive_message.rx_dlen; i++)
    {
        msg->data[i] = receive_message.rx_data[i];
    }

    return 1;
}

sdk_err_t gd32_can_write(sdk_can_t *can, sdk_can_msg_t *msg)
{
    can_trasnmit_message_struct transmit_message;

    can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    if(msg->ide == SDK_CAN_IDE_STANDARD)
    {
        transmit_message.tx_sfid = msg->canid;
        transmit_message.tx_ff = CAN_FF_STANDARD;
    }
    else if (msg->ide == SDK_CAN_IDE_EXTENDED)
    {
        transmit_message.tx_efid = msg->canid;
        transmit_message.tx_ff = CAN_FF_EXTENDED;
    }
    if (msg->rtr == SDK_CAN_RTR_DATA)
    {
        transmit_message.tx_ft = CAN_FT_DATA;
    }
    else if (msg->rtr == SDK_CAN_RTR_REMOTE)
    {
        transmit_message.tx_ft = CAN_FT_REMOTE;
    }
    transmit_message.tx_dlen = msg->dlc;
    for(int i = 0; i < msg->dlc; i++)
    {
        transmit_message.tx_data[i] = msg->data[i];
    }
    can_message_transmit(can->instance, &transmit_message);
    return SDK_OK;
}

sdk_err_t gd32_can_control(sdk_can_t *can, int32_t cmd, void *args)
{
    return SDK_OK;
}

void CAN0_RX0_IRQHandler(void)
{
    if(can0.can_fifo_num == CAN_FIFO0)
        sdk_can_rx_isr(&can0);
}

sdk_can_t can0 = 
{
    .instance = CAN0,
    .clock = RCU_CAN0,
    .irq = CAN0_RX0_IRQn,
    .irq_prio = 0,
    .can_fifo_num = CAN_FIFO0,
    .state = CAN_UNINIT,
    .ops.open = gd32_can_open,
    .ops.close = gd32_can_close,
    .ops.read = gd32_can_read,
    .ops.write = gd32_can_write,
    .ops.control = gd32_can_control,
};
