/*!
    \file    slcd_seg.c
    \brief   source of the slcd segment driver

    \version 2021-08-04, V1.0.0, firmware for GD32L23x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/
#include "dhs_sdk.h"

#include "slcd_seg_l23x.h"
#include "gd32l23x.h"
#include "gd32l23x_driver.h"

/* digit SLCD DATA buffer */
uint8_t digit[7];

/* table of the digit code for SLCD */
__I uint32_t numbertable[] = {
    0x77 /*00:0*/, 0x12 /*01:1*/, 0x6B /*02:2*/, 0x5B /*03:3*/, 0x1E /*04:4*/, 0x5D /*05:5*/, 0x7D /*06:6*/, 0x13 /*07:7*/, 0x7F /*08:8*/, 0x5F /*09:9*/,
    0b00111111 /*10:A*/, 0b01111100 /*11:b*/, 0b01100101 /*12:C*/, 0b01111010 /*13:d*/, 0b01101101 /*14:E*/, 0b00101101 /*15:F*/, 0x00000000 /*16: */, 0b00101111 /*17:P*/, 0b00110111 /*18:N*/, 0b01100100 /*19:L*/,
    0b00001000 /*20:-*/, 0b00111110 /*21:H*/, 0b01110110 /*22:U*/, 0b00101000 /*23:r*/, 0b01111000 /*24:o*/, 0b01110000 /*25:u*/, 0b01010010 /*26:J*/, 0b00111000 /*27:n*/
};

static void digit_to_code(uint8_t c);
static void slcd_gpio_config(void);

/*!
    \brief      convert digit to SLCD code
    \param[in]  the digit to write
    \param[out] none
    \retval     none
*/
static void digit_to_code(uint8_t c)
{
    uint8_t ch = 0;

    /* the *c is a number */
    if(c < ARRAY_SIZE(numbertable)) {
        ch = numbertable[c];
    }

    for(int i = 0; i < 7; i++)
    {
        digit[i] =  (uint8_t)((ch >> i) & 0x01);
    }
}

static void rcu_configuration(void)
{
    /* enable PMU clock */
    rcu_periph_clock_enable(RCU_PMU);
    /* PMU backup domain write enable */
    pmu_backup_write_enable();

    /* enable IRC32K */
    rcu_osci_on(RCU_IRC32K);
    /* wait for IRC40K stabilization flags */
    rcu_osci_stab_wait(RCU_IRC32K);
    /* configure the RTC clock source selection */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
}


/*!
    \brief      init the GPIO port of SLCD peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void slcd_gpio_config(void)
{
    /* enable the clock of GPIO */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    /* SLCD GPIO */
    /* configure GPIOA */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_af_set(GPIOA, GPIO_AF_3, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

    /* configure GPIOB */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_af_set(GPIOB, GPIO_AF_3, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    /* configure GPIOC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    /* configure GPIOD */
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
    gpio_af_set(GPIOD, GPIO_AF_3, GPIO_PIN_2);

    gpio_mode_set(GPIOD, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_6);
}

/*!
    \brief      configure SLCD peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void slcd_seg_configuration(void)
{
    volatile uint16_t i;

    rcu_configuration();
    /* configure the SLCD GPIO pins */
    slcd_gpio_config();

    /* enable the clock of SLCD */
    rcu_periph_clock_enable(RCU_SLCD);
    /* wait 2 RTC clock to write SLCD register */
    for(i = 0; i < 1000; i++);

    slcd_deinit();
	
    slcd_com_seg_remap(DISABLE);

    /* config the prescaler and the divider of SLCD clock */
    slcd_clock_config(SLCD_PRESCALER_2, SLCD_DIVIDER_16);
    /* SLCD bias voltage select */
    slcd_bias_voltage_select(SLCD_BIAS_1_3);
    /* SLCD duty cycle select */
    slcd_duty_select(SLCD_DUTY_1_8); 
    /* SLCD voltage source select */
    slcd_voltage_source_select(SLCD_VOLTAGE_EXTERNAL);
    /* SLCD pulse on duration config */
    slcd_pulse_on_duration_config(SLCD_PULSEON_DURATION_5);
    /* SLCD dead time duration config */
    slcd_dead_time_config(SLCD_DEADTIME_PERIOD_7);
    slcd_contrast_ratio_config(SLCD_CONTRAST_LEVEL_3);
    /* enable the permanent high drive */
    slcd_high_drive_config(ENABLE);

    slcd_enhance_mode_enable();
		

    /* wait for SLCD CFG register synchronization */
    while(!slcd_flag_get(SLCD_FLAG_SYN));
    /* enable SLCD interface */
    slcd_enable();
    /* wait for SLCD controller on flag */
    while(!slcd_flag_get(SLCD_FLAG_ON));
}

void slcd_icon_display(int ch, int disp)
{
    /* wait the last SLCD DATA update request finished */
    while (slcd_flag_get(SLCD_FLAG_UPR))
        ;

    switch (ch)
    {
    case LEIJI: // 累积用量字符 T2
    {
        if (disp)
            SLCD_DATA5 |= (uint32_t)(0x4000000);//com5,seg26
        else
            SLCD_DATA5 &= ~(uint32_t)(0x4000000);
        break;
    }
    case BIAOHAO: // 表号 T3
    {
        if (disp)
            SLCD_DATA6 |= (uint32_t)(0x4000000);//com6,seg26
        else
            SLCD_DATA6 &= ~(uint32_t)(0x4000000);
        break;
    }
    case TONGXIN: //通信 T4
    {
        if (disp)
            SLCD_DATA7 |= (uint32_t)(0x4000000);//com7,seg26
        else
            SLCD_DATA7 &= ~(uint32_t)(0x4000000);
        break;
    }
    case YICHANG: //异常 T5
    {
        if (disp)
            SLCD_DATA7 |= (uint32_t)(0x8000000);//com7,seg27
        else
            SLCD_DATA7 &= ~(uint32_t)(0x8000000);
        break;
    }
    case FAGUAN: //阀关
    {
        if (disp){
            SLCD_DATA5 |= (uint32_t)(0x8000000);//T7 seg27
            SLCD_DATA6 |= (uint32_t)(0x8000000);//T6
        }
        else
            SLCD_DATA5 &= ~(uint32_t)(0x8000000);
        break;
    }
    case QIANYA: //欠压 T19
    {
        if (disp)
            SLCD_DATA0 |= (uint32_t)(0x8000000);//com0,seg27
        else
            SLCD_DATA0 &= ~(uint32_t)(0x8000000);
        break;
    }
    case XIAOSHUDIAN: //小数点
    {
        if (disp)
        {
            SLCD_DATA3 |= (uint32_t)(0x1000);//T12
            SLCD_DATA3 |= (uint32_t)(0x4000);//T13
            SLCD_DATA3 |= (uint32_t)(0x8000);//T14
            SLCD_DATA3 |= (uint32_t)(0x2000000);//T15
        }
        else
        {
            SLCD_DATA3 &= ~(uint32_t)(0x1000);
            SLCD_DATA3 &= ~(uint32_t)(0x4000);
            SLCD_DATA3 &= ~(uint32_t)(0x8000);
            SLCD_DATA3 &= ~(uint32_t)(0x2000000);
        }
        break;
    }
    case m3: //T18
    {
        if (disp)
            SLCD_DATA1 |= (uint32_t)(0x8000000);//com1,seg27
        else
            SLCD_DATA1 &= ~(uint32_t)(0x8000000);
        break;
    }
    case RIQI: //日期 T8
    {
        if (disp)
            SLCD_DATA7 |= (uint32_t)(0x20); //com7,seg5
        else
            SLCD_DATA7 &= ~(uint32_t)(0x20);
        break;
    }
    case SHIJIAN: //时间 T9
    {
        if (disp)
            SLCD_DATA6 |= (uint32_t)(0x20); //com6,seg5
        else
            SLCD_DATA6 &= ~(uint32_t)(0x20);
        break;
    }
    case XINHAO: //信号 T20
    {
        if (disp)
            SLCD_DATA3 |= (uint32_t)(0x8000000); //com3,seg27
        else
            SLCD_DATA3 &= ~(uint32_t)(0x8000000);
        break;
    }
    case BIANKUANG: //边框 T1
    {
        if (disp)
            SLCD_DATA3 |= (uint32_t)(0x4000000); //com3,seg26
        else
            SLCD_DATA3 &= ~(uint32_t)(0x4000000);
        break;
    }
    default:
        break;
    }
}

/*!
    \brief      write one digit to the SLCD DATA register
    \param[in]  ch: the digit to write
    \param[in]  position: position in the SLCD of the digit to write
    \param[out] none
    \retval     none
*/
void slcd_seg_digit_display(uint8_t ch, uint8_t position)
{
    /* wait the last SLCD DATA update request finished */
    while(slcd_flag_get(SLCD_FLAG_UPR));

    /* SLCD write a char */
    slcd_seg_digit_write(ch, position);

    /* request SLCD DATA update */
    slcd_data_update_request();
}

/*!
    \brief      write a integer(6 digits) to SLCD DATA register
    \param[in]  num: number to send to SLCD(0-99999999)
    \param[out] none
    \retval     none
*/
void slcd_seg_number_display(uint32_t num)
{
    uint8_t i = 0x00, length, ch[8];

    ch[7] = num / 10000000;
    ch[6] = (num % 10000000) / 1000000;
    ch[5] = (num % 1000000) / 100000;
    ch[4] = (num % 100000) / 10000;
    ch[3] = (num % 10000) / 1000;
    ch[2] = (num % 1000) / 100;
    ch[1] = (num % 100) / 10;
    ch[0] = num % 10;
    
    if(ch[7]){
        length = 8;
    } else if(ch[6]) {
        length = 7;
    } else if(ch[5]) {
        length = 6;
    } else if(ch[4]) {
        length = 5;
    } else if(ch[3]) {
        length = 4;
    } else if(ch[2]) {
        length = 3;
    } else if(ch[1]) {
        length = 2;
    } else {
        length = 1;
    }

    slcd_seg_clear_all();
    /* wait the last SLCD DATA update request finished */
    while(slcd_flag_get(SLCD_FLAG_UPR));

    /* send the string character one by one to SLCD */
    while(i < length) {
        /* display one digit on SLCD */
        slcd_seg_digit_write(ch[i], 8 - i);
        /* increment the digit counter */
        i++;
    }

    /* request SLCD DATA update */
    slcd_data_update_request();
}
#if 0
/*!
    \brief      write a float number(6 digits which has 2 decimal) to SLCD DATA register
    \param[in]  num: number to send to SLCD
    \param[out] none
    \retval     none
*/
void slcd_seg_floatnumber_display(float num)
{
    uint8_t i = 0x00, length, ch[6];
    uint32_t temp;

    temp = (uint32_t)(num * 100);
    ch[5] = temp / 100000;
    ch[4] = (temp % 100000) / 10000;
    ch[3] = (temp % 10000) / 1000;
    ch[2] = (temp % 1000) / 100;
    ch[1] = (temp % 100) / 10;
    ch[0] = temp % 10;

    if(ch[5]) {
        length = 6;
    } else if(ch[4]) {
        length = 5;
    } else if(ch[3]) {
        length = 4;
    } else {
        length = 3;
    }

    slcd_seg_clear_all();
    /* wait the last SLCD DATA update request finished */
    while(slcd_flag_get(SLCD_FLAG_UPR));

    /* send the string character one by one to SLCD */
    while(i < length) {
        /* display one digit on SLCD */
        slcd_seg_digit_write(ch[i], 6 - i, FLOAT);
        /* increment the digit counter */
        i++;
    }

    /* request SLCD DATA update */
    slcd_data_update_request();
}
#endif
/*!
    \brief      write a digit to SLCD DATA register
    \param[in]  ch: the digit to write
    \param[in]  position: position in the SLCD of the digit to write,which can be 1..8
    \param[in]  type: the type of the data
    \param[out] none
    \retval     none
*/
void slcd_seg_digit_write(uint8_t ch, uint8_t position)
{
    /* wait the last SLCD DATA update request finished */
    while (slcd_flag_get(SLCD_FLAG_UPR))
    ;
    /* convert ASCii to SLCD digit or char */
    digit_to_code(ch);
   
    switch (position)
    {
    case 1:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG6) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFFFBF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFFFBF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 6);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 6);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 6);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 6);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 6);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 6);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 6);
        break;
        
    case 2:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG10) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFFBFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFFBFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 10);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 10);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 10);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 10);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 10);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 10);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 10);
        break;

    case 3:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG11) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFF7FF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFF7FF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 11);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 11);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 11);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 11);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 11);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 11);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 11);
        break;

    case 4:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG12) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFEFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFEFFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 12);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 12);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 12);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 12);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 12);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 12);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 12);
        break;

    case 5:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG13) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFDFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFDFFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 13);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 13);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 13);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 13);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 13);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 13);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 13);
        break;

    case 6:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG14) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFBFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFBFFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 14);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 14);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 14);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 14);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 14);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 14);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 14);
        break;

    case 7:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG15) */
        SLCD_DATA7 &= (uint32_t)(0xFFFF7FFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFF7FFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 15);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 15);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 15);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 15);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 15);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 15);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 15);
        break;

    case 8:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG25) */
        SLCD_DATA7 &= (uint32_t)(0xFDFFFFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFDFFFFFF); //

        /* write the corresponding segments */
        SLCD_DATA7 |= (uint32_t)(digit[0] << 25);
        SLCD_DATA6 |= (uint32_t)(digit[1] << 25);
        SLCD_DATA5 |= (uint32_t)(digit[2] << 25);
        SLCD_DATA4 |= (uint32_t)(digit[3] << 25);
        SLCD_DATA2 |= (uint32_t)(digit[4] << 25);
        SLCD_DATA1 |= (uint32_t)(digit[5] << 25);
        SLCD_DATA0 |= (uint32_t)(digit[6] << 25);
        break;
    }
}

/*!
    \brief      clear data in the SLCD DATA register
    \param[in]  position: position in the SLCD of the digit to write,which can be 1..6
    \param[out] none
    \retval     none
*/
void slcd_seg_digit_clear(uint8_t position)
{
    /* wait the last SLCD DATA update request finished */
    while (slcd_flag_get(SLCD_FLAG_UPR))
        ;
    switch(position) {
    case 1:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG6) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFFFBF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFFFBF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFFFBF); //
        break;
		
    case 2:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG10) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFFBFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFFBFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFFBFF); //
        break;
		
    case 3:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG11) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFF7FF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFF7FF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFF7FF); //
        break;

    case 4:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG12) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFEFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFEFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFEFFF); //
        break;

    case 5:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG13) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFDFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFDFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFDFFF); //
        break;

    case 6:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG14) */
        SLCD_DATA7 &= (uint32_t)(0xFFFFBFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFFBFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFFBFFF); //
        break;

    case 7:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG15) */
        SLCD_DATA7 &= (uint32_t)(0xFFFF7FFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA5 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA4 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA2 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA1 &= (uint32_t)(0xFFFF7FFF); //
        SLCD_DATA0 &= (uint32_t)(0xFFFF7FFF); //
        break;

    case 8:
        /* clear the corresponding segments (COM7,6,5,4,2,0,1,SEG25) */
        SLCD_DATA7 &= (uint32_t)(0xFDFFFFFF); // SLCD_DATA7 > COM7
        SLCD_DATA6 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA5 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA4 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA2 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA1 &= (uint32_t)(0xFDFFFFFF); //
        SLCD_DATA0 &= (uint32_t)(0xFDFFFFFF); //
        break;
    }
}

/*!
    \brief      clear all the SLCD DATA register
    \param[in]  none
    \param[out] none
    \retval     none
*/
void slcd_seg_clear_all(void)
{
    /* wait the last SLCD DATA update request finished */
    while (slcd_flag_get(SLCD_FLAG_UPR))
        ;
    SLCD_DATA0 = 0x00000000;
    SLCD_DATA1 = 0x00000000;
    SLCD_DATA2 = 0x00000000;
    SLCD_DATA3 = 0x00000000;
    SLCD_DATA4 = 0x00000000;
    SLCD_DATA5 = 0x00000000;
    SLCD_DATA6 = 0x00000000;
    SLCD_DATA7 = 0x00000000;
}

void slcd_seg_full_screen(void)
{
    /* wait the last SLCD DATA update request finished */
    while (slcd_flag_get(SLCD_FLAG_UPR))
        ;
    SLCD_DATA0 = 0xFFFFFFFF;
    SLCD_DATA1 = 0xFFFFFFFF;
    SLCD_DATA2 = 0xFFFFFFFF;
    SLCD_DATA3 = 0xFFFFFFFF;
    SLCD_DATA4 = 0xFFFFFFFF;
    SLCD_DATA5 = 0xFFFFFFFF;
    SLCD_DATA6 = 0xFFFFFFFF;
    SLCD_DATA7 = 0xFFFFFFFF;
}
