/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_rtc.h"
#include "dhs_sdk.h"
#include "sdk_board.h"

#define DBG_TAG "bsp.rtc"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

#if !defined(SDK_RTC_CLOCK_SELECT_LSI) && \
    !defined(SDK_RTC_CLOCK_SELECT_LSE)
#define SDK_RTC_CLOCK_SELECT_LSI
#endif

#define BKP_VALUE    0x32F0

static volatile uint32_t prescaler_a = 0, prescaler_s = 0;

#if defined(SOC_SERIES_GD32L23x)
static time_t get_rtc_timestamp(void)
{
    rtc_parameter_struct   rtc_initpara;
    struct tm tm_new;

    rtc_current_time_get(&rtc_initpara);

    tm_new.tm_sec  = bcd2dec(rtc_initpara.second);
    tm_new.tm_min  = bcd2dec(rtc_initpara.minute);
    tm_new.tm_hour = bcd2dec(rtc_initpara.hour);
    tm_new.tm_mday = bcd2dec(rtc_initpara.date);
    tm_new.tm_mon  = bcd2dec(rtc_initpara.month) - 1;
    tm_new.tm_year = bcd2dec(rtc_initpara.year) + 100;

    return mktime(&tm_new);
}

static int32_t set_rtc_timestamp(time_t time_stamp)
{
    rtc_parameter_struct   rtc_initpara;
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return -SDK_ERROR;
    }

    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn  = prescaler_s;

    rtc_initpara.second      = dec2bcd(p_tm->tm_sec       );
    rtc_initpara.minute      = dec2bcd(p_tm->tm_min       );
    rtc_initpara.hour        = dec2bcd(p_tm->tm_hour      );
    rtc_initpara.date        = dec2bcd(p_tm->tm_mday      );
    rtc_initpara.month       = dec2bcd(p_tm->tm_mon  + 1  );
    rtc_initpara.year        = dec2bcd(p_tm->tm_year - 100);
    if(dec2bcd(p_tm->tm_wday) == 0)
        rtc_initpara.day_of_week = 7;
    else
        rtc_initpara.day_of_week = dec2bcd(p_tm->tm_wday);
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_AM;

    rtc_init(&rtc_initpara);

    return SDK_OK;
}
#elif defined (SOC_SERIES_GD32F3X0)
static time_t get_rtc_timestamp(void)
{
    rtc_parameter_struct   rtc_initpara;
    struct tm tm_new;

    rtc_current_time_get(&rtc_initpara);

    tm_new.tm_sec  = bcd2dec(rtc_initpara.rtc_second);
    tm_new.tm_min  = bcd2dec(rtc_initpara.rtc_minute);
    tm_new.tm_hour = bcd2dec(rtc_initpara.rtc_hour);
    tm_new.tm_mday = bcd2dec(rtc_initpara.rtc_date);
    tm_new.tm_mon  = bcd2dec(rtc_initpara.rtc_month) - 1;
    tm_new.tm_year = bcd2dec(rtc_initpara.rtc_year) + 100;

    return mktime(&tm_new);
}

static int32_t set_rtc_timestamp(time_t time_stamp)
{
    rtc_parameter_struct   rtc_initpara;
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return -SDK_ERROR;
    }

    rtc_initpara.rtc_factor_asyn = prescaler_a;
    rtc_initpara.rtc_factor_syn  = prescaler_s;

    rtc_initpara.rtc_second      = dec2bcd(p_tm->tm_sec       );
    rtc_initpara.rtc_minute      = dec2bcd(p_tm->tm_min       );
    rtc_initpara.rtc_hour        = dec2bcd(p_tm->tm_hour      );
    rtc_initpara.rtc_date        = dec2bcd(p_tm->tm_mday      );
    rtc_initpara.rtc_month       = dec2bcd(p_tm->tm_mon  + 1  );
    rtc_initpara.rtc_year        = dec2bcd(p_tm->tm_year - 100);
    rtc_initpara.rtc_day_of_week = dec2bcd(p_tm->tm_wday + 1  );

    rtc_initpara.rtc_display_format = RTC_24HOUR;
    rtc_initpara.rtc_am_pm = RTC_AM;

    rtc_init(&rtc_initpara);

    return SDK_OK;
}
#else
static time_t get_rtc_timestamp(void)
{
    time_t rtc_counter;

    rtc_counter = (time_t)rtc_counter_get();

    return rtc_counter;
}

static int32_t set_rtc_timestamp(time_t time_stamp)
{
    uint32_t rtc_counter;

    rtc_counter = (uint32_t)time_stamp;

    /* wait until LWOFF bit in RTC_CTL to 1 */
    rtc_lwoff_wait();
    /* enter configure mode */
    rtc_configuration_mode_enter();
    /* write data to rtc register */
    rtc_counter_set(rtc_counter);
    /* exit configure mode */
    rtc_configuration_mode_exit();
    /* wait until LWOFF bit in RTC_CTL to 1 */
    rtc_lwoff_wait();

    return SDK_OK;
}
#endif

static sdk_err_t gd32_rtc_control(sdk_rtc_t *rtc, int32_t cmd, void *args)
{
    sdk_err_t result = SDK_OK;
#if defined(RT_USING_ALARM)
    struct rt_rtc_wkalarm *wkalarm;
    rtc_alarm_struct rtc_alarm;
#endif

    switch (cmd)
    {
    case SDK_DRIVER_RTC_GET_TIME:
        *(uint32_t *)args = get_rtc_timestamp();
        break;

    case SDK_DRIVER_RTC_SET_TIME:
        if (set_rtc_timestamp(*(uint32_t *)args))
        {
            result = -SDK_ERROR;
        }
        
#ifdef RT_USING_ALARM
        rt_alarm_dump();
        rt_alarm_update(dev, 1);
        rt_alarm_dump();
#endif
        break;
#if defined(RT_USING_ALARM)
    case SDK_DRIVER_RTC_GET_ALARM:
        wkalarm = (struct rt_rtc_wkalarm *)args;
        rtc_alarm_get(RTC_ALARM0, &rtc_alarm);

        wkalarm->tm_hour = bcd2dec(rtc_alarm.alarm_hour);
        wkalarm->tm_min = bcd2dec(rtc_alarm.alarm_minute);
        wkalarm->tm_sec = bcd2dec(rtc_alarm.alarm_second);
        LOG_D("RTC: get rtc_alarm time : hour: %d , min: %d , sec:  %d \n",
              wkalarm->tm_hour,
              wkalarm->tm_min,
              wkalarm->tm_sec);
        break;

    case SDK_DRIVER_RTC_SET_ALARM:
        wkalarm = (struct rt_rtc_wkalarm *)args;

        rtc_alarm_disable(RTC_ALARM0);
        rtc_alarm.alarm_mask = RTC_ALARM_DATE_MASK;
        rtc_alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
        rtc_alarm.alarm_day = 0x31;
        rtc_alarm.am_pm = RTC_AM;

        rtc_alarm.alarm_hour = dec2bcd(wkalarm->tm_hour);
        rtc_alarm.alarm_minute = dec2bcd(wkalarm->tm_min);
        rtc_alarm.alarm_second = dec2bcd(wkalarm->tm_sec);

        rtc_alarm_config(RTC_ALARM0, &rtc_alarm);

        rtc_interrupt_enable(RTC_INT_ALARM0);
        rtc_alarm_enable(RTC_ALARM0);
        nvic_irq_enable(RTC_Alarm_IRQn, 0U);
        LOG_D("RTC: set rtc_alarm time : hour: %d , min: %d , sec:  %d \n",
              wkalarm->tm_hour,
              wkalarm->tm_min,
              wkalarm->tm_sec);
        break;
#endif
    default:
      return -(SDK_E_INVALID);
    }

    return result;
}

static void rtc_pre_config(void)
{
#if defined(SDK_RTC_CLOCK_SELECT_LSI) && !defined(SOC_SERIES_GD32L23x)
    rcu_osci_on(RCU_IRC40K);
    rcu_osci_stab_wait(RCU_IRC40K);
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
    prescaler_s = 0x18F;
    prescaler_a = 0x63;
#elif defined(SDK_RTC_CLOCK_SELECT_LSI) && defined(SOC_SERIES_GD32L23x)
    rcu_osci_on(RCU_IRC32K);
    rcu_osci_stab_wait(RCU_IRC32K);
    rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
    prescaler_s = 0x13F;
    prescaler_a = 0x63;
#elif defined (SDK_RTC_CLOCK_SELECT_LSE)
    rcu_osci_on(RCU_LXTAL);
    rcu_osci_stab_wait(RCU_LXTAL);
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    prescaler_s = 0xFF;
    prescaler_a = 0x7F;
#else
#error RTC clock source should be defined.
#endif /* SDK_RTC_CLOCK_SELECT_LSI */

    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
}

void WakeupConfig1Hz()
{
    rtc_wakeup_disable();
    rtc_flag_clear(RTC_FLAG_WT);
    exti_flag_clear(EXTI_20);

    exti_interrupt_flag_clear(EXTI_20);
    nvic_irq_enable(RTC_WKUP_IRQn, 3);
    exti_init(EXTI_20, EXTI_INTERRUPT, EXTI_TRIG_RISING);

    rtc_wakeup_clock_set(WAKEUP_RTCCK_DIV16);
    rtc_wakeup_timer_set(2000);
    rtc_wakeup_enable();
    rtc_interrupt_enable(RTC_INT_WAKEUP);
}

static void rtc_setup(void)
{
    time_t rtc_counter = 1652906863;

    set_rtc_timestamp(rtc_counter);
#if defined (SOC_SERIES_GD32F3X0) || defined(SOC_SERIES_GD32L23x)
    RTC_BKP0 = BKP_VALUE;
#else
    bkp_write_data(BKP_DATA_0, BKP_VALUE);
#endif
}

#if defined(RT_USING_ALARM)
void RTC_Alarm_IRQHandler(void)
{
    if (RESET != rtc_flag_get(RTC_FLAG_ALARM0))
    {
        rtc_flag_clear(RTC_FLAG_ALARM0);
        exti_flag_clear(EXTI_17);
        
        rt_alarm_update(&g_gd32_rtc_dev.rtc_dev, 1);
    }
}
#endif

static sdk_err_t gd32_rtc_open(sdk_rtc_t *rtc)
{
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
#if defined(SOC_SERIES_GD32L23x)
    rcu_periph_clock_enable(RCU_BKP);
    rtc_register_sync_wait();
#elif defined (SOC_SERIES_GD32F3X0)
    rtc_register_sync_wait();
#else
    rcu_periph_clock_enable(RCU_BKPI);
#endif

    rtc_pre_config();
//    WakeupConfig1Hz();
#if defined (SOC_SERIES_GD32F3X0) || defined(SOC_SERIES_GD32L23x)
    if (BKP_VALUE != RTC_BKP0)
#else
    if (bkp_read_data(BKP_DATA_0) != BKP_VALUE)
#endif
    {
        rtc_setup();
        LOG_D("rtc_setup....\n\r");
    }
    else
    {
        /* detect the reset source */
        if (RESET != rcu_flag_get(RCU_FLAG_PORRST))
        {
            LOG_D("power on reset occurred....\n\r");
        }
        else if (RESET != rcu_flag_get(RCU_FLAG_EPRST))
        {
            LOG_D("external reset occurred....\n\r");
        }
        LOG_D("no need to configure RTC....\n\r");
#if defined(RT_USING_ALARM)
        rtc_flag_clear(RTC_STAT_ALRM0F);
        exti_flag_clear(EXTI_17);
#endif
    }
    rcu_all_reset_flag_clear();
#if defined(RT_USING_ALARM)
    /* RTC alarm interrupt configuration */
    exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    nvic_irq_enable(RTC_Alarm_IRQn, 0);
#endif

    return SDK_OK;
}

static sdk_err_t gd32_rtc_close(sdk_rtc_t *rtc)
{
    return SDK_OK;
}

__WEAK void sdk_rtc_wakeup_callback(void)
{
    LOG_D("\n");
}


void RTC_WKUP_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_WT) != RESET)
    {
        exti_interrupt_flag_clear(EXTI_20);
        rtc_flag_clear(RTC_FLAG_WT);

        //INT callback
        sdk_rtc_wakeup_callback();
        
    }
}

sdk_rtc_t rtc = 
{
    .ops.open = gd32_rtc_open,
    .ops.close = gd32_rtc_close,
    .ops.control = gd32_rtc_control,
};
