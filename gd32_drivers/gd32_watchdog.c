/**
 * Copyright (c) 2023 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_board.h"

#define DBG_TAG "bsp.wdt"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

typedef struct {
    uint32_t min_threshold_s;
    uint32_t max_threshold_s;
    uint32_t current_threshold_s;
} gd32_wdt_device_t;

static gd32_wdt_device_t g_wdt_dev;

#if defined(SOC_SERIES_GD32L23x)
#define RCU_IRC_WDT_TYPE   RCU_IRC32K
#define RCU_IRC_WDT_VALUE  (32000UL)
#else
#define RCU_IRC_WDT_TYPE   RCU_IRC40K
#define RCU_IRC_WDT_VALUE  (40000UL)
#endif

static sdk_err_t gd32_wdt_open(sdk_watchdog_t *wdt)
{

    rcu_osci_on(RCU_IRC_WDT_TYPE);
    if (ERROR == rcu_osci_stab_wait(RCU_IRC_WDT_TYPE))
    {
        LOG_E("failed init IRC40K clock for free watchdog.");
        return -SDK_E_INVALID;
    }

    g_wdt_dev.min_threshold_s = 1;
    g_wdt_dev.max_threshold_s = (0xfff << 8) / RCU_IRC_WDT_VALUE;
    LOG_I("threshold section [%u, %d]", \
        g_wdt_dev.min_threshold_s, g_wdt_dev.max_threshold_s);

    fwdgt_write_enable();
    fwdgt_config(0xfff, FWDGT_PSC_DIV256);
    fwdgt_enable();

    return 0;
}

static sdk_err_t gd32_wdt_control(sdk_watchdog_t *wdt, int cmd, void *arg)
{
    uint32_t param;
    int flag_status;
    uint32_t timeout = FWDGT_RLD_TIMEOUT;

    switch (cmd)
    {
    case SDK_DRIVER_WDT_KEEPALIVE:
        fwdgt_counter_reload();
        break;
    case SDK_DRIVER_WDT_SET_TIMEOUT:
        param = *(uint32_t *) arg;
        if ((param > g_wdt_dev.max_threshold_s) || \
            (param < g_wdt_dev.min_threshold_s))
        {
            LOG_E("invalid param@%u.", param);
            return -SDK_E_INVALID;
        }
        else
        {
            g_wdt_dev.current_threshold_s = param;
        }
        fwdgt_write_enable();
        fwdgt_reload_value_config(param * RCU_IRC_WDT_VALUE >> 8);
        timeout = FWDGT_RLD_TIMEOUT;
        /* wait until the RUD flag to be reset */
        do {
            flag_status = fwdgt_flag_get(FWDGT_STAT_RUD);
        } while((--timeout > 0U) && ((uint32_t)RESET != flag_status));

        fwdgt_counter_reload();

        break;
    case SDK_DRIVER_WDT_GET_TIMEOUT:
        *(uint32_t *)arg = g_wdt_dev.current_threshold_s;
        break;
    case SDK_DRIVER_WDT_START:
        fwdgt_enable();
        break;
    default:
        LOG_W("This command is not supported.");
        return -SDK_ERROR;
    }

    return SDK_OK;
}

sdk_watchdog_t wdt = 
{
    .ops.open = gd32_wdt_open,
    .ops.control = gd32_wdt_control,
};
