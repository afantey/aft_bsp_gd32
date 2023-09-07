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

#define MCU_FLASH_START_ADRESS     ((uint32_t)0x08000000)
#define MCU_FLASH_SIZE             (128 * 1024)
#define MCU_FLASH_END_ADDRESS      ((uint32_t)(MCU_FLASH_START_ADRESS + MCU_FLASH_SIZE))
#define MCU_FLASH_PAGE_SIZE        ((uint16_t)0x400)

#define ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t addr)
{
    uint32_t page = 0;
    page = ALIGN_DOWN(addr - MCU_FLASH_START_ADRESS, MCU_FLASH_PAGE_SIZE) + MCU_FLASH_START_ADRESS;
    return page;
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

    while (addr < end_addr)
    {
        fmc_state = fmc_word_program(addr, *((uint32_t *)buf));
        /* Clear All pending flags */
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
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

    if ((addr + size) > MCU_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%08x)\n", (void *)(addr + size));
        return -SDK_E_INVALID;
    }

    sdk_hw_interrupt_disable();
    fmc_unlock();

    uint32_t address = 0;
    int NbPages = (size + MCU_FLASH_PAGE_SIZE - 1) / MCU_FLASH_PAGE_SIZE;
    uint32_t PageAddress = GetPage(addr);
    fmc_state_enum fmc_state = FMC_READY;

    /* Clear All pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* Erase page by page to be done*/
    for (address = PageAddress; address < ((NbPages * MCU_FLASH_PAGE_SIZE) + PageAddress); address += MCU_FLASH_PAGE_SIZE)
    {
        fmc_state = fmc_page_erase(address);
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
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

sdk_flash_t gd32f30x_onchip_flash = 
{
    .ops.open = gd32_flash_open,
    .ops.close = gd32_flash_close,
    .ops.read = gd32_flash_read,
    .ops.write = gd32_flash_write,
    .ops.erase = gd32_flash_erase,
    .ops.control = gd32_flash_control,
};
