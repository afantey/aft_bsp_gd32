/**
 * Copyright (c) 2022 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_gpio.h"
#include "dhs_sdk.h"
#include "board.h"

#define SDK_GPIO_SET(port, pin, set) do{if(set)GPIO_BOP(port) = (uint32_t)pin;else GPIO_BC(port) = (uint32_t)pin; }while(0);

void sdk_gpio_write_pin(char port, uint32_t pin, uint8_t set)
{
    switch(port)
    {
        case 'A':
            SDK_GPIO_SET(GPIOA, pin, set);
            break;
        case 'B':
            SDK_GPIO_SET(GPIOB, pin, set);
            break;
        case 'C':
            SDK_GPIO_SET(GPIOB, pin, set);
            break;
        case 'D':
            SDK_GPIO_SET(GPIOB, pin, set);
            break;
    }
   
}