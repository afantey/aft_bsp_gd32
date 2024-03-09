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


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = CTL_SECTOR_NUMBER_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = CTL_SECTOR_NUMBER_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = CTL_SECTOR_NUMBER_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = CTL_SECTOR_NUMBER_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = CTL_SECTOR_NUMBER_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = CTL_SECTOR_NUMBER_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = CTL_SECTOR_NUMBER_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = CTL_SECTOR_NUMBER_7;
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = CTL_SECTOR_NUMBER_8;
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = CTL_SECTOR_NUMBER_9;
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = CTL_SECTOR_NUMBER_10;
    }
    else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
    {
        sector = CTL_SECTOR_NUMBER_11;
    }
    else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
    {
        sector = CTL_SECTOR_NUMBER_12;
    }
    else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
    {
        sector = CTL_SECTOR_NUMBER_13;
    }
    else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
    {
        sector = CTL_SECTOR_NUMBER_14;
    }
    else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
    {
        sector = CTL_SECTOR_NUMBER_15;
    }
    else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
    {
        sector = CTL_SECTOR_NUMBER_16;
    }
    else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
    {
        sector = CTL_SECTOR_NUMBER_17;
    }
    else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
    {
        sector = CTL_SECTOR_NUMBER_18;
    }
    else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
    {
        sector = CTL_SECTOR_NUMBER_19;
    }
    else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
    {
        sector = CTL_SECTOR_NUMBER_20;
    }
    else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
    {
        sector = CTL_SECTOR_NUMBER_21;
    }
    else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
    {
        sector = CTL_SECTOR_NUMBER_22;
    }
    else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
    {
        sector = CTL_SECTOR_NUMBER_23;
    }
    return sector;
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
    uint32_t FirstSector = 0, NbOfSectors = 0;

    if ((addr + size) > MCU_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%08x)\n", (void *)(addr + size));
        return -SDK_E_INVALID;
    }

    sdk_hw_interrupt_disable();
    fmc_unlock();
    /* Get the 1st sector to erase */
    FirstSector = GetSector(addr);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(addr + size - 1) - FirstSector + 1;

    fmc_state_enum fmc_state = FMC_READY;

    uint32_t sector_index = 0U;
    /* Erase page by page to be done*/
    for (sector_index = FirstSector; sector_index < (NbOfSectors + FirstSector); sector_index++)
    {
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
        fmc_state = fmc_sector_erase(sector_index);
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
