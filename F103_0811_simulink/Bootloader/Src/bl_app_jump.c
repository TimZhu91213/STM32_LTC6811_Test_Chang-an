#include "bl_app_jump.h"
#include "bl_config.h"
#include "stm32f1xx_hal.h"

typedef void (*pFunction)(void);

bool bl_app_is_valid(void)
{
    uint32_t sp = *(volatile uint32_t *)BL_APP_START_ADDR;
    uint32_t rv = *(volatile uint32_t *)(BL_APP_START_ADDR + 4u);

    if ((sp & 0xFFF00000u) != 0x20000000u) {
        return false;
    }
    if ((rv & 0xFF000000u) != 0x08000000u) {
        return false;
    }
    if ((rv & 1u) == 0u) {
        return false;
    }
    return true;
}

void bl_app_jump(void)
{
    uint32_t app_sp;
    uint32_t app_reset;
    pFunction app_entry;
    volatile uint32_t *dst;
    const uint32_t *src;
    uint32_t i;

    if (!bl_app_is_valid()) {
        return;
    }

    app_sp = *(volatile uint32_t *)BL_APP_START_ADDR;
    app_reset = *(volatile uint32_t *)(BL_APP_START_ADDR + 4u);
    app_entry = (pFunction)app_reset;

#if BL_NEEDS_SRAM_VECTOR_REMAP
    /* Copy app vectors to SRAM and remap 0x00000000 -> SRAM (F103 has no VTOR).
     * MEM_MODE[1:0] @ AFIO_MAPR bits1:0 (RM0008); CMSIS may omit these macros. */
    dst = (volatile uint32_t *)0x20000000u;
    src = (const uint32_t *)BL_APP_START_ADDR;
    for (i = 0; i < 48u; i++) {
        dst[i] = src[i];
    }
    __HAL_RCC_AFIO_CLK_ENABLE();
    {
      uint32_t mapr = AFIO->MAPR;
      mapr &= ~0x3u;   /* clear MEM_MODE */
      mapr |= 0x3u;    /* 11: Embedded SRAM mapped at 0x00000000 */
      AFIO->MAPR = mapr;
    }
#endif

    __disable_irq();
    for (i = 0; i < 8u; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFu;
        NVIC->ICPR[i] = 0xFFFFFFFFu;
    }

    __set_MSP(app_sp);
    app_entry();
}
