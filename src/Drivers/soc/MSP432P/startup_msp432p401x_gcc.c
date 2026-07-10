#include <stdint.h>

#define WDT_A_BASE      0x40004800UL
#define WDT_A_CTL       (*(volatile uint16_t *)(WDT_A_BASE + 0x0C))
#define WDT_A_HOLD      0x0080
#define WDT_A_PW        0x5A00

extern int main(void);
extern void SystemInit(void);
extern void __libc_init_array(void);

extern uint32_t __StackTop;
extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

typedef void (*pFunc)(void);

void Reset_Handler(void);
void HardFault_Handler(void);
void Default_Handler(void);

void NMI_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)     __attribute__((weak, alias("Default_Handler")));

void PSS_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void CS_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void PCM_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void WDT_A_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void FLCTL_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void COMP_E0_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void COMP_E1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void TA0_0_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA0_N_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA1_0_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA1_N_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA2_0_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA2_N_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA3_0_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TA3_N_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void EUSCIA0_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIA1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIA2_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIA3_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIB0_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIB1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIB2_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void EUSCIB3_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void ADC14_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void T32_INT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void T32_INT2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void T32_INTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void AES256_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void RTC_C_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA_ERR_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void DMA_INT3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA_INT2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA_INT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA_INT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PORT1_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void PORT2_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void PORT3_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void PORT4_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void PORT5_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void PORT6_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));

__attribute__((used, section(".isr_vector")))
const pFunc interruptVectors[] =
{
    (pFunc)&__StackTop,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    PSS_IRQHandler,
    CS_IRQHandler,
    PCM_IRQHandler,
    WDT_A_IRQHandler,
    FPU_IRQHandler,
    FLCTL_IRQHandler,
    COMP_E0_IRQHandler,
    COMP_E1_IRQHandler,
    TA0_0_IRQHandler,
    TA0_N_IRQHandler,
    TA1_0_IRQHandler,
    TA1_N_IRQHandler,
    TA2_0_IRQHandler,
    TA2_N_IRQHandler,
    TA3_0_IRQHandler,
    TA3_N_IRQHandler,
    EUSCIA0_IRQHandler,
    EUSCIA1_IRQHandler,
    EUSCIA2_IRQHandler,
    EUSCIA3_IRQHandler,
    EUSCIB0_IRQHandler,
    EUSCIB1_IRQHandler,
    EUSCIB2_IRQHandler,
    EUSCIB3_IRQHandler,
    ADC14_IRQHandler,
    T32_INT1_IRQHandler,
    T32_INT2_IRQHandler,
    T32_INTC_IRQHandler,
    AES256_IRQHandler,
    RTC_C_IRQHandler,
    DMA_ERR_IRQHandler,
    DMA_INT3_IRQHandler,
    DMA_INT2_IRQHandler,
    DMA_INT1_IRQHandler,
    DMA_INT0_IRQHandler,
    PORT1_IRQHandler,
    PORT2_IRQHandler,
    PORT3_IRQHandler,
    PORT4_IRQHandler,
    PORT5_IRQHandler,
    PORT6_IRQHandler
};

void Reset_Handler(void)
{
    WDT_A_CTL = WDT_A_PW | WDT_A_HOLD;

    uint32_t *src = &__data_load__;
    uint32_t *dst = &__data_start__;

    while (dst < &__data_end__)
    {
        *dst++ = *src++;
    }

    for (uint32_t *p = &__bss_start__; p < &__bss_end__; )
    {
        *p++ = 0;
    }

    SystemInit();
    
    main();

    while (1) {}
}

void Default_Handler(void)
{
    while (1);
}

void HardFault_Handler()
{
    while (1);
}
