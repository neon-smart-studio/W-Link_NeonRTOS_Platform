#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#ifdef DEVICE_TIMSPM0

bool gpio_pin_init_status[hwGPIO_Pin_MAX] = {false};

static hwGPIO_Direction gpio_current_dir[hwGPIO_Pin_MAX] = {hwGPIO_Direction_Input};
static hwGPIO_Pull_Mode gpio_current_mode[hwGPIO_Pin_MAX] = {hwGPIO_Pull_Mode_None};
static hwGPIO_Interrupt_Mode gpio_current_irq_mode[hwGPIO_Int_Pin_MAX] = {hwGPIO_Interrupt_Mode_MAX};
static GPIO_Interrupt_Event_Handler gpio_irq_handlers[hwGPIO_Int_Pin_MAX] = {NULL};

hwGPIO_OpResult GPIO_Pin_Init(hwGPIO_Pin pin, hwGPIO_Direction dir, hwGPIO_Pull_Mode pull_mode)
{
    DL_GPIO_RESISTOR resistor;

    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (dir >= hwGPIO_Direction_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (pull_mode >= hwGPIO_Pull_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (pull_mode == hwGPIO_Pull_Mode_OpenDrain)
    {
        return hwGPIO_Unsupport;
    }

    if (gpio_pin_init_status[pin])
    {
        return hwGPIO_PinConflict;
    }

    switch (pull_mode)
    {
        case hwGPIO_Pull_Mode_None:
            resistor = DL_GPIO_RESISTOR_NONE;
            break;

        case hwGPIO_Pull_Mode_Up:
            resistor = DL_GPIO_RESISTOR_PULL_UP;
            break;

        case hwGPIO_Pull_Mode_Down:
            resistor = DL_GPIO_RESISTOR_PULL_DOWN;
            break;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);
    uint32_t iomux = GPIO_Map_Soc_Int_Pin_IOMUX(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if(iomux==GPIO_SOC_IOMUX_INVALID)
    {
        return hwGPIO_InvalidParameter;
    }

    /*
     * Output 保留輸入 buffer，因此仍可 GPIO_Pin_Read()。
     * Output_Only 則關閉輸入 buffer。
     */
    if (dir == hwGPIO_Direction_Output_Only)
    {
        DL_GPIO_initDigitalOutput(iomux);
    }
    else
    {
        DL_GPIO_initDigitalInput(iomux);
    }

    DL_GPIO_setDigitalInternalResistor(iomux, resistor);

    if (dir == hwGPIO_Direction_Input)
    {
        DL_GPIO_disableOutput(port, pin_mask);
    }
    else
    {
        DL_GPIO_enableOutput(port, pin_mask);
    }

    gpio_current_dir[pin]  = dir;
    gpio_current_mode[pin] = pull_mode;
    gpio_pin_init_status[pin] = true;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_DeInit(hwGPIO_Pin pin)
{
    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (!gpio_pin_init_status[pin])
    {
        return hwGPIO_OK;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);
    uint32_t iomux = GPIO_Map_Soc_Int_Pin_IOMUX(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if(iomux==GPIO_SOC_IOMUX_INVALID)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_disableOutput(port, pin_mask);

    DL_GPIO_initDigitalInput(iomux);

    DL_GPIO_setDigitalInternalResistor(iomux, DL_GPIO_RESISTOR_NONE);

    gpio_current_dir[pin] = hwGPIO_Direction_Input;
    gpio_current_mode[pin] = hwGPIO_Pull_Mode_None;
    gpio_pin_init_status[pin] = false;

    return hwGPIO_OK;
}

bool GPIO_Pin_is_Init(hwGPIO_Pin pin)
{
    return gpio_pin_init_status[pin];
}

hwGPIO_OpResult GPIO_Pin_Set_Direction(hwGPIO_Pin pin, hwGPIO_Direction dir)
{
    DL_GPIO_RESISTOR resistor;

    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (dir >= hwGPIO_Direction_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (gpio_current_dir[pin] ==
        hwGPIO_Direction_Output_Only)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);
    uint32_t iomux = GPIO_Map_Soc_Int_Pin_IOMUX(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if(iomux==GPIO_SOC_IOMUX_INVALID)
    {
        return hwGPIO_InvalidParameter;
    }

    switch (gpio_current_mode[pin])
    {
        case hwGPIO_Pull_Mode_None:
            resistor = DL_GPIO_RESISTOR_NONE;
            break;

        case hwGPIO_Pull_Mode_Up:
            resistor = DL_GPIO_RESISTOR_PULL_UP;
            break;

        case hwGPIO_Pull_Mode_Down:
            resistor = DL_GPIO_RESISTOR_PULL_DOWN;
            break;
    }

    /*
     * Output 保留輸入 buffer，因此仍可 GPIO_Pin_Read()。
     * Output_Only 則關閉輸入 buffer。
     */
    if (dir == hwGPIO_Direction_Output_Only)
    {
        DL_GPIO_initDigitalOutput(iomux);
    }
    else
    {
        DL_GPIO_initDigitalInput(iomux);
    }

    DL_GPIO_setDigitalInternalResistor(iomux, resistor);

    if (dir == hwGPIO_Direction_Input)
    {
        DL_GPIO_disableOutput(port, pin_mask);
    }
    else
    {
        DL_GPIO_enableOutput(port, pin_mask);
    }

    gpio_current_dir[pin] = dir;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_Direction(hwGPIO_Pin pin, hwGPIO_Direction *dir)
{
    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if(dir == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    *dir = gpio_current_dir[pin];

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Set_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode pull_mode)
{
    DL_GPIO_RESISTOR resistor;

    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (pull_mode >= hwGPIO_Pull_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (pull_mode == hwGPIO_Pull_Mode_OpenDrain)
    {
        return hwGPIO_Unsupport;
    }

    uint32_t iomux = GPIO_Map_Soc_Int_Pin_IOMUX(pin);

    if(iomux==GPIO_SOC_IOMUX_INVALID)
    {
        return hwGPIO_InvalidParameter;
    }

    switch (pull_mode)
    {
        case hwGPIO_Pull_Mode_None:
            resistor = DL_GPIO_RESISTOR_NONE;
            break;

        case hwGPIO_Pull_Mode_Up:
            resistor = DL_GPIO_RESISTOR_PULL_UP;
            break;

        case hwGPIO_Pull_Mode_Down:
            resistor = DL_GPIO_RESISTOR_PULL_DOWN;
            break;
    }

    DL_GPIO_setDigitalInternalResistor(iomux, resistor);

    gpio_current_mode[pin] = pull_mode;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Get_PullMode(hwGPIO_Pin pin, hwGPIO_Pull_Mode *pull_mode)
{
    if (!GPIO_IsValidPin(pin) || pull_mode == NULL)
        return hwGPIO_InvalidParameter;

    *pull_mode = gpio_current_mode[pin];

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Read(hwGPIO_Pin pin, bool *level)
{
    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (level == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if (gpio_current_dir[pin] == hwGPIO_Direction_Output_Only)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    *level = (DL_GPIO_readPins(port, pin_mask) != 0U);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Write(hwGPIO_Pin pin, bool level)
{
    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (gpio_current_dir[pin] == hwGPIO_Direction_Input)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if (level)
    {
        DL_GPIO_setPins(port, pin_mask);
    }
    else
    {
        DL_GPIO_clearPins(port, pin_mask);
    }

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Pin_Toggle(hwGPIO_Pin pin)
{
    if(pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (gpio_current_dir[pin] == hwGPIO_Direction_Input)
    {
        return hwGPIO_Unsupport;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_togglePins(port, pin_mask);

    return hwGPIO_OK;
}

/*
 * POLARITY15_0 / POLARITY31_16 每個 pin 使用兩個 bit。
 * 必須 read-modify-write，否則設定一個 pin 會蓋掉同 port
 * 其他 pin 的中斷模式。
 */
static hwGPIO_OpResult GPIO_SetInterruptPolarity(
    GPIO_Regs *port,
    uint32_t pin_mask,
    hwGPIO_Interrupt_Mode mode)
{
    uint32_t pin_index;
    uint32_t local_index;
    uint32_t shift;
    uint32_t field_mask;
    uint32_t edge_value;
    uint32_t polarity;

    if (port == NULL ||
        pin_mask == 0U ||
        (pin_mask & (pin_mask - 1U)) != 0U)
    {
        return hwGPIO_InvalidParameter;
    }

    while ((pin_mask & (1UL << pin_index)) == 0U)
    {
        pin_index++;
    }

    local_index = pin_index & 0x0FU;
    shift       = local_index * 2U;
    field_mask  = 0x03UL << shift;

    if (pin_index < 16U)
    {
        switch (mode)
        {
            case hwGPIO_Interrupt_Mode_Rising_Edge:
                edge_value = DL_GPIO_PIN_0_EDGE_RISE;
                break;

            case hwGPIO_Interrupt_Mode_Falling_Edge:
                edge_value = DL_GPIO_PIN_0_EDGE_FALL;
                break;

            case hwGPIO_Interrupt_Mode_Both_Edge:
                edge_value = DL_GPIO_PIN_0_EDGE_RISE_FALL;
                break;

            default:
                return hwGPIO_InvalidParameter;
        }

        polarity = DL_GPIO_getLowerPinsPolarity(port);

        polarity &= ~field_mask;
        polarity |= (edge_value << shift) & field_mask;

        DL_GPIO_setLowerPinsPolarity(port, polarity);
    }
    else
    {
        switch (mode)
        {
            case hwGPIO_Interrupt_Mode_Rising_Edge:
                edge_value = DL_GPIO_PIN_16_EDGE_RISE;
                break;

            case hwGPIO_Interrupt_Mode_Falling_Edge:
                edge_value = DL_GPIO_PIN_16_EDGE_FALL;
                break;

            case hwGPIO_Interrupt_Mode_Both_Edge:
                edge_value = DL_GPIO_PIN_16_EDGE_RISE_FALL;
                break;

            default:
                return hwGPIO_InvalidParameter;
        }

        polarity = DL_GPIO_getUpperPinsPolarity(port);

        polarity &= ~field_mask;
        polarity |= (edge_value << shift) & field_mask;

        DL_GPIO_setUpperPinsPolarity(port, polarity);
    }

    return hwGPIO_OK;
}

static void GPIO_IRQ_HandlePort(GPIO_Regs *port)
{
    uint32_t flags;

    if (port == NULL)
        return;

    flags = DL_GPIO_getEnabledInterruptStatus(port, UINT32_MAX);
    if (flags == 0U)
    {
        return;
    }

    /*
     * 先清除這次取得的 pending bits，再呼叫 callback。
     * callback 執行期間產生的新 edge 可再次觸發 IRQ。
     */
    DL_GPIO_clearInterruptStatus(port, flags);

    while (flags != 0U)
    {
        uint32_t int_mask;
        hwGPIO_Int_Pin int_pin;
        hwGPIO_Interrupt_Action action;

        int_mask = flags & (0U - flags);
        flags &= ~int_mask;

        int_pin = GPIO_Map_Int_Pin_By_Mask(port, int_mask);

        if (!GPIO_IsValidIntPin(int_pin))
            continue;

        switch (gpio_current_irq_mode[int_pin])
        {
            case hwGPIO_Interrupt_Mode_Rising_Edge:
                action = hwGPIO_Interrupt_Action_Rising_Edge;
                break;

            case hwGPIO_Interrupt_Mode_Falling_Edge:
                action = hwGPIO_Interrupt_Action_Falling_Edge;
                break;

            case hwGPIO_Interrupt_Mode_Both_Edge:
                /*
                 * MSPM0 IRQ status 不直接告知是哪一個 edge。
                 * edge 後為 high 視為 rising，為 low 視為 falling。
                 */
                if (DL_GPIO_readPins(port, int_mask) != 0U)
                {
                    action = hwGPIO_Interrupt_Action_Rising_Edge;
                }
                else
                {
                    action = hwGPIO_Interrupt_Action_Falling_Edge;
                }
                break;

            default:
                continue;
        }

        if (gpio_irq_handlers[int_pin] != NULL)
        {
            gpio_irq_handlers[int_pin](int_pin, action);
        }
    }
}

/*
 * MSPM0 多數型號的 GPIOA/GPIOB IRQ 共用 GROUP1_IRQHandler。
 * 如果其他模組也需要 GROUP1，應把以下內容合併到專案的
 * 共用 GROUP1 dispatcher，不能同時定義兩個 GROUP1_IRQHandler。
 */
void GROUP1_IRQHandler(void)
{
#if defined(GPIOA_BASE)
    GPIO_IRQ_HandlePort(GPIOA_BASE);
#endif

#if defined(GPIOB_BASE)
    GPIO_IRQ_HandlePort(GPIOB_BASE);
#endif

#if defined(GPIOC_BASE)
    GPIO_IRQ_HandlePort(GPIOC_BASE);
#endif
}

hwGPIO_OpResult GPIO_Interrupt_Init(
    hwGPIO_Int_Pin irq_pin,
    hwGPIO_Interrupt_Mode mode)
{    hwGPIO_OpResult result;

    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (mode >= hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (gpio_pin_init_status[irq_pin])
    {
        return hwGPIO_PinConflict;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);
    uint32_t iomux = GPIO_Map_Soc_Int_Pin_IOMUX(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    if(iomux==GPIO_SOC_IOMUX_INVALID)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_initDigitalInput(iomux);

    DL_GPIO_setDigitalInternalResistor(iomux, DL_GPIO_RESISTOR_NONE);

    DL_GPIO_disableOutput(port, pin_mask);

    result = GPIO_SetInterruptPolarity(port, pin_mask, mode);
    if (result != hwGPIO_OK)
    {
        return result;
    }

    DL_GPIO_disableInterrupt(port, pin_mask);

    DL_GPIO_clearInterruptStatus(port, pin_mask);

    gpio_current_irq_mode[irq_pin] = mode;
    gpio_current_dir[irq_pin] = hwGPIO_Direction_Input;
    gpio_current_mode[irq_pin] = hwGPIO_Pull_Mode_None;
    gpio_pin_init_status[irq_pin] = true;

    DL_GPIO_enableInterrupt(port, pin_mask);

    /*
     * GPIOA_INT_IRQn、GPIOB_INT_IRQn 在 GROUP1 架構下通常是
     * 同一個 NVIC IRQ number，重複 EnableIRQ 沒有副作用。
     */
    NVIC_EnableIRQ(GPIOA_INT_IRQn);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_DeInit(hwGPIO_Int_Pin irq_pin)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (!gpio_pin_init_status[irq_pin])
    {
        return hwGPIO_OK;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_disableInterrupt(port, pin_mask);

    DL_GPIO_clearInterruptStatus(port, pin_mask);

    gpio_current_irq_mode[irq_pin] = hwGPIO_Interrupt_Mode_MAX;
    gpio_pin_init_status[irq_pin] = false;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Config_Interrupt_Mode(hwGPIO_Int_Pin irq_pin, hwGPIO_Interrupt_Mode mode)
{
    hwGPIO_OpResult result;

    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (mode >= hwGPIO_Interrupt_Mode_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_disableInterrupt(port, pin_mask);

    result = GPIO_SetInterruptPolarity(port, pin_mask, mode);
    if (result != hwGPIO_OK)
    {
        return result;
    }

    DL_GPIO_clearInterruptStatus(port, pin_mask);

    gpio_current_irq_mode[irq_pin] = mode;

    DL_GPIO_enableInterrupt(port, pin_mask);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Register_Interrupt_Handler(hwGPIO_Int_Pin irq_pin, GPIO_Interrupt_Event_Handler handler)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    gpio_irq_handlers[irq_pin] = handler;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Unregister_Interrupt_Handler(hwGPIO_Int_Pin irq_pin)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    gpio_irq_handlers[irq_pin] = NULL;

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Enable(hwGPIO_Int_Pin irq_pin)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_clearInterruptStatus(port, pin_mask);

    DL_GPIO_enableInterrupt(port, pin_mask);

    NVIC_EnableIRQ(GPIOA_INT_IRQn);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Disable(hwGPIO_Int_Pin irq_pin)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    DL_GPIO_disableInterrupt(port, pin_mask);

    DL_GPIO_clearInterruptStatus(port, pin_mask);

    return hwGPIO_OK;
}

hwGPIO_OpResult GPIO_Interrupt_Pin_Read(hwGPIO_Int_Pin irq_pin, bool *level)
{
    if (irq_pin >= hwGPIO_Int_Pin_MAX)
    {
        return hwGPIO_InvalidParameter;
    }

    if (level == NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    GPIO_Regs *port = GPIO_Map_Soc_Int_Port_Base(irq_pin);
    uint32_t pin_mask = GPIO_Map_Soc_Int_Pin_Mask(irq_pin);

    if(port==NULL)
    {
        return hwGPIO_InvalidParameter;
    }

    *level = (DL_GPIO_readPins(port, pin_mask) != 0U);

    return hwGPIO_OK;
}

#endif /* DEVICE_TIMSPM0 */