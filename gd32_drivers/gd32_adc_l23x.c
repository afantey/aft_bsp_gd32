/**
 * Copyright (c) 2023 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 * 2023.6.2       rgw          add adc_channel_16_to_19 enable
 */

#include "sdk_adc.h"
#include "dhs_sdk.h"
#include "sdk_board.h"

#define DBG_TAG "bsp.adc"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

static void rcu_config(void)
{
    /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);
}

static void gpio_config(void)
{
    /* config the GPIO as analog mode */
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);
    // gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_2 | GPIO_PIN_3);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_3);
}

static void adc_config(void)
{
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL, 1U);

    /* ADC trigger config */
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_NONE);
    /* ADC external trigger config */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);

    adc_channel_16_to_19(ADC_TEMP_CHANNEL_SWITCH, ENABLE);
    adc_channel_16_to_19(ADC_INTERNAL_CHANNEL_SWITCH, ENABLE);
    
    /* enable ADC interface */
    adc_enable();
    sdk_hw_us_delay(3U);
    /* ADC calibration and reset calibration */
    adc_calibration_enable();
}

static sdk_err_t gd32_adc_open(sdk_adc_t *adc)
{
    /* system clocks configuration */
    rcu_config();
    /* GPIO configuration */
    gpio_config();
    /* ADC configuration */
    adc_config();

    return SDK_OK;
}

static sdk_err_t gd32_adc_close(sdk_adc_t *adc)
{
    adc_deinit();

    return SDK_OK;
}

static sdk_err_t gd32_adc_read(sdk_adc_t *adc, uint32_t channel, uint32_t *value)
{
    /* ADC regular channel config */
    adc_regular_channel_config(0U, channel, ADC_SAMPLETIME_7POINT5);
    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

    /* wait the end of conversion flag */
    while(!adc_flag_get(ADC_FLAG_EOC));
    /* clear the end of conversion flag */
    adc_flag_clear(ADC_FLAG_EOC);
    /* return regular channel sample value */

    *value = adc_regular_data_read();
    return SDK_OK;
}


static int32_t gd32_adc_control(sdk_adc_t *adc, int cmd, void *args)
{
    return SDK_OK;
}

sdk_adc_t adc = 
{
    .ops.open = gd32_adc_open,
    .ops.close = gd32_adc_close,
    .ops.read = gd32_adc_read,
    .ops.control = gd32_adc_control,
};
