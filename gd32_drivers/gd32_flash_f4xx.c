/**
 * Copyright (c) 2023 Infinitech Technology Co., Ltd
 * 
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_board.h"
#include "sdk_flash.h"

#define DBG_LVL DBG_LOG
#define DBG_TAG "mcu.flash"
#include "sdk_log.h"

/* Base address of the Flash sectors Bank 1 */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */

// define from 0 to 23
#define FLASH_SECTOR_NAME_0      ((uint32_t)0U)  /*!< Sector Name 0   */
#define FLASH_SECTOR_NAME_1      ((uint32_t)1U)  /*!< Sector Name 1   */
#define FLASH_SECTOR_NAME_2      ((uint32_t)2U)  /*!< Sector Name 2   */
#define FLASH_SECTOR_NAME_3      ((uint32_t)3U)  /*!< Sector Name 3   */
#define FLASH_SECTOR_NAME_4      ((uint32_t)4U)  /*!< Sector Name 4   */
#define FLASH_SECTOR_NAME_5      ((uint32_t)5U)  /*!< Sector Name 5   */
#define FLASH_SECTOR_NAME_6      ((uint32_t)6U)  /*!< Sector Name 6   */
#define FLASH_SECTOR_NAME_7      ((uint32_t)7U)  /*!< Sector Name 7   */
#define FLASH_SECTOR_NAME_8      ((uint32_t)8U)  /*!< Sector Name 8   */
#define FLASH_SECTOR_NAME_9      ((uint32_t)9U)  /*!< Sector Name 9   */
#define FLASH_SECTOR_NAME_10     ((uint32_t)10U) /*!< Sector Name 10  */
#define FLASH_SECTOR_NAME_11     ((uint32_t)11U) /*!< Sector Name 11  */
#define FLASH_SECTOR_NAME_12     ((uint32_t)12U) /*!< Sector Name 12  */
#define FLASH_SECTOR_NAME_13     ((uint32_t)13U) /*!< Sector Name 13  */
#define FLASH_SECTOR_NAME_14     ((uint32_t)14U) /*!< Sector Name 14   */
#define FLASH_SECTOR_NAME_15     ((uint32_t)15U) /*!< Sector Name 15   */
#define FLASH_SECTOR_NAME_16     ((uint32_t)16U) /*!< Sector Name 16   */
#define FLASH_SECTOR_NAME_17     ((uint32_t)17U) /*!< Sector Name 17   */
#define FLASH_SECTOR_NAME_18     ((uint32_t)18U) /*!< Sector Name 18   */
#define FLASH_SECTOR_NAME_19     ((uint32_t)19U) /*!< Sector Name 19   */
#define FLASH_SECTOR_NAME_20     ((uint32_t)20U) /*!< Sector Name 20   */
#define FLASH_SECTOR_NAME_21     ((uint32_t)21U) /*!< Sector Name 21   */
#define FLASH_SECTOR_NAME_22     ((uint32_t)22U) /*!< Sector Name 22   */
#define FLASH_SECTOR_NAME_23     ((uint32_t)23U) /*!< Sector Name 23   */

/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t get_sector_name(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_NAME_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_NAME_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_NAME_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_NAME_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_NAME_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_NAME_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_NAME_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_SECTOR_NAME_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_SECTOR_NAME_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_SECTOR_NAME_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_SECTOR_NAME_10;
    }
    else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
    {
        sector = FLASH_SECTOR_NAME_11;
    }
    else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
    {
        sector = FLASH_SECTOR_NAME_12;
    }
    else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
    {
        sector = FLASH_SECTOR_NAME_13;
    }
    else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
    {
        sector = FLASH_SECTOR_NAME_14;
    }
    else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
    {
        sector = FLASH_SECTOR_NAME_15;
    }
    else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
    {
        sector = FLASH_SECTOR_NAME_16;
    }
    else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
    {
        sector = FLASH_SECTOR_NAME_17;
    }
    else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
    {
        sector = FLASH_SECTOR_NAME_18;
    }
    else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
    {
        sector = FLASH_SECTOR_NAME_19;
    }
    else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
    {
        sector = FLASH_SECTOR_NAME_20;
    }
    else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
    {
        sector = FLASH_SECTOR_NAME_21;
    }
    else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
    {
        sector = FLASH_SECTOR_NAME_22;
    }
    else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
    {
        sector = FLASH_SECTOR_NAME_23;
    }
    return sector;
}

/*!
    \brief      get the sector number by a given sector name
    \param[in]  address: a given sector name
    \param[out] none
    \retval     uint32_t: sector number
*/
uint32_t sector_name_to_number(uint32_t sector_name)
{
    if(11 >= sector_name){
        return CTL_SN(sector_name);
    }else if(23 >= sector_name){
        return CTL_SN(sector_name + 4);
    }else if(27 >= sector_name){
        return CTL_SN(sector_name - 12);
    }else{
        while(1);
    }
}


sdk_err_t gd32_flash_open(sdk_flash_t *flash)
{
    fmc_unlock();
    return SDK_OK;
}

sdk_err_t gd32_flash_close(sdk_flash_t *flash)
{
    fmc_lock();
    return SDK_OK;
}

int32_t gd32_flash_read(sdk_flash_t *flash, uint32_t addr, uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > MCU_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (0x%08x)", (void *)(addr + size));
        return -SDK_E_INVALID;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

int32_t gd32_flash_write(sdk_flash_t *flash, uint32_t addr, const uint8_t *buf, size_t size)
{
    sdk_err_t result = SDK_OK;
    fmc_state_enum fmc_state = FMC_READY;
    uint32_t end_addr = addr + size;

    if (addr % 4 != 0)
    {
        LOG_E("write addr must be 4-byte alignment");
        return -SDK_E_INVALID;
    }

    if ((end_addr) > MCU_FLASH_END_ADDRESS)
    {
        LOG_E("write outrange flash size! addr is (0x%08x)", (void *)(addr + size));
        return -SDK_E_INVALID;
    }

    sdk_hw_interrupt_disable();
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    while (addr < end_addr)
    {
        fmc_state = fmc_word_program(addr, *((uint32_t *)buf));
        if(fmc_state == FMC_READY)
        {
            if (*(uint32_t *)addr != *(uint32_t *)buf)
            {
                LOG_E("ERROR: write read! addr is (0x%08x), write is %d, read is %d\n", (void *)(addr), *(uint32_t *)addr, *(uint32_t *)buf);
                result = -SDK_ERROR;
                break;
            }
            addr += 4;
            buf  += 4;
        }
        else
        {
            LOG_E("ERROR: write! addr is (0x%08x)\n", (void *)(addr));
            result = -SDK_ERROR;
            break;
        }
    }

    fmc_lock();
    sdk_hw_interrupt_enable();

    if (result != SDK_OK)
    {
        return result;
    }

    return size;
}

sdk_err_t gd32_flash_erase(sdk_flash_t *flash, uint32_t addr, size_t size)
{
    sdk_err_t result = SDK_OK;
    uint32_t first_sector_name = 0, num_of_sectors = 0;

    if ((addr + size) > MCU_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%08x)\n", (void *)(addr + size));
        return -SDK_E_INVALID;
    }

    sdk_hw_interrupt_disable();
    fmc_unlock();
    /* Get the 1st sector to erase */
    first_sector_name = get_sector_name(addr);
    /* Get the number of sector to erase from 1st sector*/
    num_of_sectors = get_sector_name(addr + size - 1) - first_sector_name + 1;

    fmc_state_enum fmc_state = FMC_READY;

    uint32_t sector_name = 0U;
    /* Erase page by page to be done*/
    for (sector_name = first_sector_name; sector_name < (num_of_sectors + first_sector_name); sector_name++)
    {
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
        fmc_state = fmc_sector_erase(sector_name_to_number(sector_name));
        if(fmc_state != FMC_READY)
        {
            result = -SDK_ERROR;
            goto __exit;
        }
    }

__exit:
    fmc_lock();
    sdk_hw_interrupt_enable();

    if (result != SDK_OK)
    {
        return result;
    }

    LOG_D("erase done: addr (0x%08x), size %d\n", (void *)addr, size);
    return size;
}

sdk_err_t gd32_flash_control(sdk_flash_t *flash, int32_t cmd, void *args)
{
    switch (cmd)
    {
    default:
        break;
    }

    return SDK_OK;
}

sdk_flash_t gd32_onchip_flash = 
{
    .ops.open = gd32_flash_open,
    .ops.close = gd32_flash_close,
    .ops.read = gd32_flash_read,
    .ops.write = gd32_flash_write,
    .ops.erase = gd32_flash_erase,
    .ops.control = gd32_flash_control,
};
