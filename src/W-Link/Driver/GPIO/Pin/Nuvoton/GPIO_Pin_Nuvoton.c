#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_NUVOTON

#include "GPIO_Pin_Nuvoton.h"

GPIO_T * GPIO_Map_Soc_Base(hwGPIO_Pin pin)
{
    GPIO_T * socBase = NULL;
    switch(pin)
    {
        case hwGPIO_Pin_A0:
        case hwGPIO_Pin_A1:
        case hwGPIO_Pin_A2:
        case hwGPIO_Pin_A3:
        case hwGPIO_Pin_A4:
        case hwGPIO_Pin_A5:
        case hwGPIO_Pin_A6:
        case hwGPIO_Pin_A7:
        case hwGPIO_Pin_A8:
        case hwGPIO_Pin_A9:
        case hwGPIO_Pin_A10:
        case hwGPIO_Pin_A11:
        case hwGPIO_Pin_A12:
        case hwGPIO_Pin_A13:
        case hwGPIO_Pin_A14:
        case hwGPIO_Pin_A15:
            socBase = GPA;
            break;
        case hwGPIO_Pin_B0:
        case hwGPIO_Pin_B1:
        case hwGPIO_Pin_B2:
        case hwGPIO_Pin_B3:
        case hwGPIO_Pin_B4:
        case hwGPIO_Pin_B5:
        case hwGPIO_Pin_B6:
        case hwGPIO_Pin_B7:
        case hwGPIO_Pin_B8:
        case hwGPIO_Pin_B9:
        case hwGPIO_Pin_B10:
        case hwGPIO_Pin_B11:
        case hwGPIO_Pin_B12:
        case hwGPIO_Pin_B13:
        case hwGPIO_Pin_B14:
        case hwGPIO_Pin_B15:
            socBase = GPB;
            break;
        case hwGPIO_Pin_C0:
        case hwGPIO_Pin_C1:
        case hwGPIO_Pin_C2:
        case hwGPIO_Pin_C3:
        case hwGPIO_Pin_C4:
        case hwGPIO_Pin_C5:
        case hwGPIO_Pin_C6:
        case hwGPIO_Pin_C7:
        case hwGPIO_Pin_C8:
        case hwGPIO_Pin_C9:
        case hwGPIO_Pin_C10:
        case hwGPIO_Pin_C11:
        case hwGPIO_Pin_C12:
        case hwGPIO_Pin_C13:
        case hwGPIO_Pin_C14:
        case hwGPIO_Pin_C15:
            socBase =GPC;
            break;
        case hwGPIO_Pin_D0:
        case hwGPIO_Pin_D1:
        case hwGPIO_Pin_D2:
        case hwGPIO_Pin_D3:
        case hwGPIO_Pin_D4:
        case hwGPIO_Pin_D5:
        case hwGPIO_Pin_D6:
        case hwGPIO_Pin_D7:
        case hwGPIO_Pin_D8:
        case hwGPIO_Pin_D9:
        case hwGPIO_Pin_D10:
        case hwGPIO_Pin_D11:
        case hwGPIO_Pin_D12:
        case hwGPIO_Pin_D13:
        case hwGPIO_Pin_D14:
        case hwGPIO_Pin_D15:
            socBase = GPD;
            break;
        case hwGPIO_Pin_E0:
        case hwGPIO_Pin_E1:
        case hwGPIO_Pin_E2:
        case hwGPIO_Pin_E3:
        case hwGPIO_Pin_E4:
        case hwGPIO_Pin_E5:
        case hwGPIO_Pin_E6:
        case hwGPIO_Pin_E7:
        case hwGPIO_Pin_E8:
        case hwGPIO_Pin_E9:
        case hwGPIO_Pin_E10:
        case hwGPIO_Pin_E11:
        case hwGPIO_Pin_E12:
        case hwGPIO_Pin_E13:
        case hwGPIO_Pin_E14:
        case hwGPIO_Pin_E15:
            socBase = GPE;
            break;
        case hwGPIO_Pin_F0:
        case hwGPIO_Pin_F1:
        case hwGPIO_Pin_F2:
        case hwGPIO_Pin_F3:
        case hwGPIO_Pin_F4:
        case hwGPIO_Pin_F5:
        case hwGPIO_Pin_F6:
        case hwGPIO_Pin_F7:
        case hwGPIO_Pin_F8:
        case hwGPIO_Pin_F9:
        case hwGPIO_Pin_F10:
        case hwGPIO_Pin_F11:
        case hwGPIO_Pin_F12:
        case hwGPIO_Pin_F13:
        case hwGPIO_Pin_F14:
        case hwGPIO_Pin_F15:
            socBase = GPF;
            break;
        case hwGPIO_Pin_G0:
        case hwGPIO_Pin_G1:
        case hwGPIO_Pin_G2:
        case hwGPIO_Pin_G3:
        case hwGPIO_Pin_G4:
        case hwGPIO_Pin_G5:
        case hwGPIO_Pin_G6:
        case hwGPIO_Pin_G7:
        case hwGPIO_Pin_G8:
        case hwGPIO_Pin_G9:
        case hwGPIO_Pin_G10:
        case hwGPIO_Pin_G11:
        case hwGPIO_Pin_G12:
        case hwGPIO_Pin_G13:
        case hwGPIO_Pin_G15:
            socBase = GPG;
            break;
        case hwGPIO_Pin_H0:
        case hwGPIO_Pin_H1:
        case hwGPIO_Pin_H2:
        case hwGPIO_Pin_H3:
        case hwGPIO_Pin_H4:
        case hwGPIO_Pin_H5:
        case hwGPIO_Pin_H6:
        case hwGPIO_Pin_H7:
        case hwGPIO_Pin_H8:
        case hwGPIO_Pin_H9:
        case hwGPIO_Pin_H10:
        case hwGPIO_Pin_H11:
        case hwGPIO_Pin_H12:
        case hwGPIO_Pin_H13:
        case hwGPIO_Pin_H14:
        case hwGPIO_Pin_H15:
            socBase = GPH;
            break;
        case hwGPIO_Pin_I0:
        case hwGPIO_Pin_I1:
        case hwGPIO_Pin_I2:
        case hwGPIO_Pin_I3:
        case hwGPIO_Pin_I4:
        case hwGPIO_Pin_I5:
        case hwGPIO_Pin_I6:
        case hwGPIO_Pin_I7:
        case hwGPIO_Pin_I8:
        case hwGPIO_Pin_I9:
        case hwGPIO_Pin_I10:
        case hwGPIO_Pin_I11:
        case hwGPIO_Pin_I12:
        case hwGPIO_Pin_I13:
        case hwGPIO_Pin_I14:
        case hwGPIO_Pin_I15:
            socBase = GPI;
            break;
    }

    return socBase;
}

uint16_t GPIO_Map_Soc_Pin(hwGPIO_Pin pin)
{
    uint16_t socPin = 0;

    switch(pin)
    {
        case hwGPIO_Pin_A0:
        case hwGPIO_Pin_B0:
        case hwGPIO_Pin_C0:
        case hwGPIO_Pin_D0:
        case hwGPIO_Pin_E0:
        case hwGPIO_Pin_F0:
        case hwGPIO_Pin_G0:
        case hwGPIO_Pin_H0:
        case hwGPIO_Pin_I0:
            socPin = BIT0;
            break;

        case hwGPIO_Pin_A1:
        case hwGPIO_Pin_B1:
        case hwGPIO_Pin_C1:
        case hwGPIO_Pin_D1:
        case hwGPIO_Pin_E1:
        case hwGPIO_Pin_F1:
        case hwGPIO_Pin_G1:
        case hwGPIO_Pin_H1:
        case hwGPIO_Pin_I1:
            socPin = BIT1;
            break;

        case hwGPIO_Pin_A2:
        case hwGPIO_Pin_B2:
        case hwGPIO_Pin_C2:
        case hwGPIO_Pin_D2:
        case hwGPIO_Pin_E2:
        case hwGPIO_Pin_F2:
        case hwGPIO_Pin_G2:
        case hwGPIO_Pin_H2:
        case hwGPIO_Pin_I2:
            socPin = BIT2;
            break;

        case hwGPIO_Pin_A3:
        case hwGPIO_Pin_B3:
        case hwGPIO_Pin_C3:
        case hwGPIO_Pin_D3:
        case hwGPIO_Pin_E3:
        case hwGPIO_Pin_F3:
        case hwGPIO_Pin_G3:
        case hwGPIO_Pin_H3:
        case hwGPIO_Pin_I3:
            socPin = BIT3;
            break;

        case hwGPIO_Pin_A4:
        case hwGPIO_Pin_B4:
        case hwGPIO_Pin_C4:
        case hwGPIO_Pin_D4:
        case hwGPIO_Pin_E4:
        case hwGPIO_Pin_F4:
        case hwGPIO_Pin_G4:
        case hwGPIO_Pin_H4:
        case hwGPIO_Pin_I4:
            socPin = BIT4;
            break;

        case hwGPIO_Pin_A5:
        case hwGPIO_Pin_B5:
        case hwGPIO_Pin_C5:
        case hwGPIO_Pin_D5:
        case hwGPIO_Pin_E5:
        case hwGPIO_Pin_F5:
        case hwGPIO_Pin_G5:
        case hwGPIO_Pin_H5:
        case hwGPIO_Pin_I5:
            socPin = BIT5;
            break;

        case hwGPIO_Pin_A6:
        case hwGPIO_Pin_B6:
        case hwGPIO_Pin_C6:
        case hwGPIO_Pin_D6:
        case hwGPIO_Pin_E6:
        case hwGPIO_Pin_F6:
        case hwGPIO_Pin_G6:
        case hwGPIO_Pin_H6:
        case hwGPIO_Pin_I6:
            socPin = BIT6;
            break;

        case hwGPIO_Pin_A7:
        case hwGPIO_Pin_B7:
        case hwGPIO_Pin_C7:
        case hwGPIO_Pin_D7:
        case hwGPIO_Pin_E7:
        case hwGPIO_Pin_F7:
        case hwGPIO_Pin_G7:
        case hwGPIO_Pin_H7:
        case hwGPIO_Pin_I7:
            socPin = BIT7;
            break;

        case hwGPIO_Pin_A8:
        case hwGPIO_Pin_B8:
        case hwGPIO_Pin_C8:
        case hwGPIO_Pin_D8:
        case hwGPIO_Pin_E8:
        case hwGPIO_Pin_F8:
        case hwGPIO_Pin_G8:
        case hwGPIO_Pin_H8:
        case hwGPIO_Pin_I8:
            socPin = BIT8;
            break;

        case hwGPIO_Pin_A9:
        case hwGPIO_Pin_B9:
        case hwGPIO_Pin_C9:
        case hwGPIO_Pin_D9:
        case hwGPIO_Pin_E9:
        case hwGPIO_Pin_F9:
        case hwGPIO_Pin_G9:
        case hwGPIO_Pin_H9:
        case hwGPIO_Pin_I9:
            socPin = BIT9;
            break;

        case hwGPIO_Pin_A10:
        case hwGPIO_Pin_B10:
        case hwGPIO_Pin_C10:
        case hwGPIO_Pin_D10:
        case hwGPIO_Pin_E10:
        case hwGPIO_Pin_F10:
        case hwGPIO_Pin_G10:
        case hwGPIO_Pin_H10:
        case hwGPIO_Pin_I10:
            socPin = BIT10;
            break;

        case hwGPIO_Pin_A11:
        case hwGPIO_Pin_B11:
        case hwGPIO_Pin_C11:
        case hwGPIO_Pin_D11:
        case hwGPIO_Pin_E11:
        case hwGPIO_Pin_F11:
        case hwGPIO_Pin_G11:
        case hwGPIO_Pin_H11:
        case hwGPIO_Pin_I11:
            socPin = BIT11;
            break;

        case hwGPIO_Pin_A12:
        case hwGPIO_Pin_B12:
        case hwGPIO_Pin_C12:
        case hwGPIO_Pin_D12:
        case hwGPIO_Pin_E12:
        case hwGPIO_Pin_F12:
        case hwGPIO_Pin_G12:
        case hwGPIO_Pin_H12:
        case hwGPIO_Pin_I12:
            socPin = BIT12;
            break;

        case hwGPIO_Pin_A13:
        case hwGPIO_Pin_B13:
        case hwGPIO_Pin_C13:
        case hwGPIO_Pin_D13:
        case hwGPIO_Pin_E13:
        case hwGPIO_Pin_F13:
        case hwGPIO_Pin_G13:
        case hwGPIO_Pin_H13:
        case hwGPIO_Pin_I13:
            socPin = BIT13;
            break;

        case hwGPIO_Pin_A14:
        case hwGPIO_Pin_B14:
        case hwGPIO_Pin_C14:
        case hwGPIO_Pin_D14:
        case hwGPIO_Pin_E14:
        case hwGPIO_Pin_F14:
        case hwGPIO_Pin_G14:
        case hwGPIO_Pin_H14:
        case hwGPIO_Pin_I14:
            socPin = BIT14;
            break;

        case hwGPIO_Pin_A15:
        case hwGPIO_Pin_B15:
        case hwGPIO_Pin_C15:
        case hwGPIO_Pin_D15:
        case hwGPIO_Pin_E15:
        case hwGPIO_Pin_F15:
        case hwGPIO_Pin_G15:
        case hwGPIO_Pin_H15:
        case hwGPIO_Pin_I15:
            socPin = BIT15;
            break;

        default:
            break;
    }

    return socPin;
}

GPIO_T * GPIO_Int_Map_Soc_Base(hwGPIO_Int_Pin pin)
{
    GPIO_T * socBase = NULL;
    switch(pin)
    {
        case hwGPIO_Int_Pin_A0:
        case hwGPIO_Int_Pin_A1:
        case hwGPIO_Int_Pin_A2:
        case hwGPIO_Int_Pin_A3:
        case hwGPIO_Int_Pin_A4:
        case hwGPIO_Int_Pin_A5:
        case hwGPIO_Int_Pin_A6:
        case hwGPIO_Int_Pin_A7:
        case hwGPIO_Int_Pin_A8:
        case hwGPIO_Int_Pin_A9:
        case hwGPIO_Int_Pin_A10:
        case hwGPIO_Int_Pin_A11:
        case hwGPIO_Int_Pin_A12:
        case hwGPIO_Int_Pin_A13:
        case hwGPIO_Int_Pin_A14:
        case hwGPIO_Int_Pin_A15:
            socBase = GPA;
            break;
        case hwGPIO_Int_Pin_B0:
        case hwGPIO_Int_Pin_B1:
        case hwGPIO_Int_Pin_B2:
        case hwGPIO_Int_Pin_B3:
        case hwGPIO_Int_Pin_B4:
        case hwGPIO_Int_Pin_B5:
        case hwGPIO_Int_Pin_B6:
        case hwGPIO_Int_Pin_B7:
        case hwGPIO_Int_Pin_B8:
        case hwGPIO_Int_Pin_B9:
        case hwGPIO_Int_Pin_B10:
        case hwGPIO_Int_Pin_B11:
        case hwGPIO_Int_Pin_B12:
        case hwGPIO_Int_Pin_B13:
        case hwGPIO_Int_Pin_B14:
        case hwGPIO_Int_Pin_B15:
            socBase = GPB;
            break;
        case hwGPIO_Int_Pin_C0:
        case hwGPIO_Int_Pin_C1:
        case hwGPIO_Int_Pin_C2:
        case hwGPIO_Int_Pin_C3:
        case hwGPIO_Int_Pin_C4:
        case hwGPIO_Int_Pin_C5:
        case hwGPIO_Int_Pin_C6:
        case hwGPIO_Int_Pin_C7:
        case hwGPIO_Int_Pin_C8:
        case hwGPIO_Int_Pin_C9:
        case hwGPIO_Int_Pin_C10:
        case hwGPIO_Int_Pin_C11:
        case hwGPIO_Int_Pin_C12:
        case hwGPIO_Int_Pin_C13:
        case hwGPIO_Int_Pin_C14:
        case hwGPIO_Int_Pin_C15:
            socBase = GPC;
            break;
        case hwGPIO_Int_Pin_D0:
        case hwGPIO_Int_Pin_D1:
        case hwGPIO_Int_Pin_D2:
        case hwGPIO_Int_Pin_D3:
        case hwGPIO_Int_Pin_D4:
        case hwGPIO_Int_Pin_D5:
        case hwGPIO_Int_Pin_D6:
        case hwGPIO_Int_Pin_D7:
        case hwGPIO_Int_Pin_D8:
        case hwGPIO_Int_Pin_D9:
        case hwGPIO_Int_Pin_D10:
        case hwGPIO_Int_Pin_D11:
        case hwGPIO_Int_Pin_D12:
        case hwGPIO_Int_Pin_D13:
        case hwGPIO_Int_Pin_D14:
        case hwGPIO_Int_Pin_D15:
            socBase = GPD;
            break;
        case hwGPIO_Int_Pin_E0:
        case hwGPIO_Int_Pin_E1:
        case hwGPIO_Int_Pin_E2:
        case hwGPIO_Int_Pin_E3:
        case hwGPIO_Int_Pin_E4:
        case hwGPIO_Int_Pin_E5:
        case hwGPIO_Int_Pin_E6:
        case hwGPIO_Int_Pin_E7:
        case hwGPIO_Int_Pin_E8:
        case hwGPIO_Int_Pin_E9:
        case hwGPIO_Int_Pin_E10:
        case hwGPIO_Int_Pin_E11:
        case hwGPIO_Int_Pin_E12:
        case hwGPIO_Int_Pin_E13:
        case hwGPIO_Int_Pin_E14:
        case hwGPIO_Int_Pin_E15:
            socBase = GPE;
            break;
        case hwGPIO_Int_Pin_F0:
        case hwGPIO_Int_Pin_F1:
        case hwGPIO_Int_Pin_F2:
        case hwGPIO_Int_Pin_F3:
        case hwGPIO_Int_Pin_F4:
        case hwGPIO_Int_Pin_F5:
        case hwGPIO_Int_Pin_F6:
        case hwGPIO_Int_Pin_F7:
        case hwGPIO_Int_Pin_F8:
        case hwGPIO_Int_Pin_F9:
        case hwGPIO_Int_Pin_F10:
        case hwGPIO_Int_Pin_F11:
        case hwGPIO_Int_Pin_F12:
        case hwGPIO_Int_Pin_F13:
        case hwGPIO_Int_Pin_F14:
        case hwGPIO_Int_Pin_F15:
            socBase = GPF;
            break;
        case hwGPIO_Int_Pin_G0:
        case hwGPIO_Int_Pin_G1:
        case hwGPIO_Int_Pin_G2:
        case hwGPIO_Int_Pin_G3:
        case hwGPIO_Int_Pin_G4:
        case hwGPIO_Int_Pin_G5:
        case hwGPIO_Int_Pin_G6:
        case hwGPIO_Int_Pin_G7:
        case hwGPIO_Int_Pin_G8:
        case hwGPIO_Int_Pin_G9:
        case hwGPIO_Int_Pin_G10:
        case hwGPIO_Int_Pin_G11:
        case hwGPIO_Int_Pin_G12:
        case hwGPIO_Int_Pin_G13:
        case hwGPIO_Int_Pin_G15:
            socBase = GPG;
            break;
        case hwGPIO_Int_Pin_H0:
        case hwGPIO_Int_Pin_H1:
        case hwGPIO_Int_Pin_H2:
        case hwGPIO_Int_Pin_H3:
        case hwGPIO_Int_Pin_H4:
        case hwGPIO_Int_Pin_H5:
        case hwGPIO_Int_Pin_H6:
        case hwGPIO_Int_Pin_H7:
        case hwGPIO_Int_Pin_H8:
        case hwGPIO_Int_Pin_H9:
        case hwGPIO_Int_Pin_H10:
        case hwGPIO_Int_Pin_H11:
        case hwGPIO_Int_Pin_H12:
        case hwGPIO_Int_Pin_H13:
        case hwGPIO_Int_Pin_H14:
        case hwGPIO_Int_Pin_H15:
            socBase = GPH;
            break;
        case hwGPIO_Int_Pin_I0:
        case hwGPIO_Int_Pin_I1:
        case hwGPIO_Int_Pin_I2:
        case hwGPIO_Int_Pin_I3:
        case hwGPIO_Int_Pin_I4:
        case hwGPIO_Int_Pin_I5:
        case hwGPIO_Int_Pin_I6:
        case hwGPIO_Int_Pin_I7:
        case hwGPIO_Int_Pin_I8:
        case hwGPIO_Int_Pin_I9:
        case hwGPIO_Int_Pin_I10:
        case hwGPIO_Int_Pin_I11:
        case hwGPIO_Int_Pin_I12:
        case hwGPIO_Int_Pin_I13:
        case hwGPIO_Int_Pin_I14:
        case hwGPIO_Int_Pin_I15:
            socBase = GPI;
            break;
    }

    return socBase;
}

uint16_t GPIO_Int_Map_Soc_Pin(hwGPIO_Int_Pin pin)
{
    uint16_t socPin = 0;

    switch(pin)
    {
        case hwGPIO_Int_Pin_A0:
        case hwGPIO_Int_Pin_B0:
        case hwGPIO_Int_Pin_C0:
        case hwGPIO_Int_Pin_D0:
        case hwGPIO_Int_Pin_E0:
        case hwGPIO_Int_Pin_F0:
        case hwGPIO_Int_Pin_G0:
        case hwGPIO_Int_Pin_H0:
        case hwGPIO_Int_Pin_I0:
            socPin = BIT0;
            break;

        case hwGPIO_Int_Pin_A1:
        case hwGPIO_Int_Pin_B1:
        case hwGPIO_Int_Pin_C1:
        case hwGPIO_Int_Pin_D1:
        case hwGPIO_Int_Pin_E1:
        case hwGPIO_Int_Pin_F1:
        case hwGPIO_Int_Pin_G1:
        case hwGPIO_Int_Pin_H1:
        case hwGPIO_Int_Pin_I1:
            socPin = BIT1;
            break;

        case hwGPIO_Int_Pin_A2:
        case hwGPIO_Int_Pin_B2:
        case hwGPIO_Int_Pin_C2:
        case hwGPIO_Int_Pin_D2:
        case hwGPIO_Int_Pin_E2:
        case hwGPIO_Int_Pin_F2:
        case hwGPIO_Int_Pin_G2:
        case hwGPIO_Int_Pin_H2:
        case hwGPIO_Int_Pin_I2:
            socPin = BIT2;
            break;

        case hwGPIO_Int_Pin_A3:
        case hwGPIO_Int_Pin_B3:
        case hwGPIO_Int_Pin_C3:
        case hwGPIO_Int_Pin_D3:
        case hwGPIO_Int_Pin_E3:
        case hwGPIO_Int_Pin_F3:
        case hwGPIO_Int_Pin_G3:
        case hwGPIO_Int_Pin_H3:
        case hwGPIO_Int_Pin_I3:
            socPin = BIT3;
            break;

        case hwGPIO_Int_Pin_A4:
        case hwGPIO_Int_Pin_B4:
        case hwGPIO_Int_Pin_C4:
        case hwGPIO_Int_Pin_D4:
        case hwGPIO_Int_Pin_E4:
        case hwGPIO_Int_Pin_F4:
        case hwGPIO_Int_Pin_G4:
        case hwGPIO_Int_Pin_H4:
        case hwGPIO_Int_Pin_I4:
            socPin = BIT4;
            break;

        case hwGPIO_Int_Pin_A5:
        case hwGPIO_Int_Pin_B5:
        case hwGPIO_Int_Pin_C5:
        case hwGPIO_Int_Pin_D5:
        case hwGPIO_Int_Pin_E5:
        case hwGPIO_Int_Pin_F5:
        case hwGPIO_Int_Pin_G5:
        case hwGPIO_Int_Pin_H5:
        case hwGPIO_Int_Pin_I5:
            socPin = BIT5;
            break;

        case hwGPIO_Int_Pin_A6:
        case hwGPIO_Int_Pin_B6:
        case hwGPIO_Int_Pin_C6:
        case hwGPIO_Int_Pin_D6:
        case hwGPIO_Int_Pin_E6:
        case hwGPIO_Int_Pin_F6:
        case hwGPIO_Int_Pin_G6:
        case hwGPIO_Int_Pin_H6:
        case hwGPIO_Int_Pin_I6:
            socPin = BIT6;
            break;

        case hwGPIO_Int_Pin_A7:
        case hwGPIO_Int_Pin_B7:
        case hwGPIO_Int_Pin_C7:
        case hwGPIO_Int_Pin_D7:
        case hwGPIO_Int_Pin_E7:
        case hwGPIO_Int_Pin_F7:
        case hwGPIO_Int_Pin_G7:
        case hwGPIO_Int_Pin_H7:
        case hwGPIO_Int_Pin_I7:
            socPin = BIT7;
            break;

        case hwGPIO_Int_Pin_A8:
        case hwGPIO_Int_Pin_B8:
        case hwGPIO_Int_Pin_C8:
        case hwGPIO_Int_Pin_D8:
        case hwGPIO_Int_Pin_E8:
        case hwGPIO_Int_Pin_F8:
        case hwGPIO_Int_Pin_G8:
        case hwGPIO_Int_Pin_H8:
        case hwGPIO_Int_Pin_I8:
            socPin = BIT8;
            break;

        case hwGPIO_Int_Pin_A9:
        case hwGPIO_Int_Pin_B9:
        case hwGPIO_Int_Pin_C9:
        case hwGPIO_Int_Pin_D9:
        case hwGPIO_Int_Pin_E9:
        case hwGPIO_Int_Pin_F9:
        case hwGPIO_Int_Pin_G9:
        case hwGPIO_Int_Pin_H9:
        case hwGPIO_Int_Pin_I9:
            socPin = BIT9;
            break;

        case hwGPIO_Int_Pin_A10:
        case hwGPIO_Int_Pin_B10:
        case hwGPIO_Int_Pin_C10:
        case hwGPIO_Int_Pin_D10:
        case hwGPIO_Int_Pin_E10:
        case hwGPIO_Int_Pin_F10:
        case hwGPIO_Int_Pin_G10:
        case hwGPIO_Int_Pin_H10:
        case hwGPIO_Int_Pin_I10:
            socPin = BIT10;
            break;

        case hwGPIO_Int_Pin_A11:
        case hwGPIO_Int_Pin_B11:
        case hwGPIO_Int_Pin_C11:
        case hwGPIO_Int_Pin_D11:
        case hwGPIO_Int_Pin_E11:
        case hwGPIO_Int_Pin_F11:
        case hwGPIO_Int_Pin_G11:
        case hwGPIO_Int_Pin_H11:
        case hwGPIO_Int_Pin_I11:
            socPin = BIT11;
            break;

        case hwGPIO_Int_Pin_A12:
        case hwGPIO_Int_Pin_B12:
        case hwGPIO_Int_Pin_C12:
        case hwGPIO_Int_Pin_D12:
        case hwGPIO_Int_Pin_E12:
        case hwGPIO_Int_Pin_F12:
        case hwGPIO_Int_Pin_G12:
        case hwGPIO_Int_Pin_H12:
        case hwGPIO_Int_Pin_I12:
            socPin = BIT12;
            break;

        case hwGPIO_Int_Pin_A13:
        case hwGPIO_Int_Pin_B13:
        case hwGPIO_Int_Pin_C13:
        case hwGPIO_Int_Pin_D13:
        case hwGPIO_Int_Pin_E13:
        case hwGPIO_Int_Pin_F13:
        case hwGPIO_Int_Pin_G13:
        case hwGPIO_Int_Pin_H13:
        case hwGPIO_Int_Pin_I13:
            socPin = BIT13;
            break;

        case hwGPIO_Int_Pin_A14:
        case hwGPIO_Int_Pin_B14:
        case hwGPIO_Int_Pin_C14:
        case hwGPIO_Int_Pin_D14:
        case hwGPIO_Int_Pin_E14:
        case hwGPIO_Int_Pin_F14:
        case hwGPIO_Int_Pin_G14:
        case hwGPIO_Int_Pin_H14:
        case hwGPIO_Int_Pin_I14:
            socPin = BIT14;
            break;

        case hwGPIO_Int_Pin_A15:
        case hwGPIO_Int_Pin_B15:
        case hwGPIO_Int_Pin_C15:
        case hwGPIO_Int_Pin_D15:
        case hwGPIO_Int_Pin_E15:
        case hwGPIO_Int_Pin_F15:
        case hwGPIO_Int_Pin_G15:
        case hwGPIO_Int_Pin_H15:
        case hwGPIO_Int_Pin_I15:
            socPin = BIT15;
            break;

        default:
            break;
    }

    return socPin;
}

uint8_t GPIO_Int_Pin_To_Index(hwGPIO_Int_Pin pin)
{
    switch(pin)
    {
        case hwGPIO_Int_Pin_A0:
        case hwGPIO_Int_Pin_B0:
        case hwGPIO_Int_Pin_C0:
        case hwGPIO_Int_Pin_D0:
        case hwGPIO_Int_Pin_E0:
        case hwGPIO_Int_Pin_F0:
        case hwGPIO_Int_Pin_G0:
        case hwGPIO_Int_Pin_H0:
        case hwGPIO_Int_Pin_I0:
            return 0;
            break;

        case hwGPIO_Int_Pin_A1:
        case hwGPIO_Int_Pin_B1:
        case hwGPIO_Int_Pin_C1:
        case hwGPIO_Int_Pin_D1:
        case hwGPIO_Int_Pin_E1:
        case hwGPIO_Int_Pin_F1:
        case hwGPIO_Int_Pin_G1:
        case hwGPIO_Int_Pin_H1:
        case hwGPIO_Int_Pin_I1:
            return 1;
            break;

        case hwGPIO_Int_Pin_A2:
        case hwGPIO_Int_Pin_B2:
        case hwGPIO_Int_Pin_C2:
        case hwGPIO_Int_Pin_D2:
        case hwGPIO_Int_Pin_E2:
        case hwGPIO_Int_Pin_F2:
        case hwGPIO_Int_Pin_G2:
        case hwGPIO_Int_Pin_H2:
        case hwGPIO_Int_Pin_I2:
            return 2;
            break;

        case hwGPIO_Int_Pin_A3:
        case hwGPIO_Int_Pin_B3:
        case hwGPIO_Int_Pin_C3:
        case hwGPIO_Int_Pin_D3:
        case hwGPIO_Int_Pin_E3:
        case hwGPIO_Int_Pin_F3:
        case hwGPIO_Int_Pin_G3:
        case hwGPIO_Int_Pin_H3:
        case hwGPIO_Int_Pin_I3:
            return 3;
            break;
            
        case hwGPIO_Int_Pin_A4:
        case hwGPIO_Int_Pin_B4:
        case hwGPIO_Int_Pin_C4:
        case hwGPIO_Int_Pin_D4:
        case hwGPIO_Int_Pin_E4:
        case hwGPIO_Int_Pin_F4:
        case hwGPIO_Int_Pin_G4:
        case hwGPIO_Int_Pin_H4:
        case hwGPIO_Int_Pin_I4:
            return 4;
            break;

        case hwGPIO_Int_Pin_A5:
        case hwGPIO_Int_Pin_B5:
        case hwGPIO_Int_Pin_C5:
        case hwGPIO_Int_Pin_D5:
        case hwGPIO_Int_Pin_E5:
        case hwGPIO_Int_Pin_F5:
        case hwGPIO_Int_Pin_G5:
        case hwGPIO_Int_Pin_H5:
        case hwGPIO_Int_Pin_I5:
            return 5;
            break;

        case hwGPIO_Int_Pin_A6:
        case hwGPIO_Int_Pin_B6:
        case hwGPIO_Int_Pin_C6:
        case hwGPIO_Int_Pin_D6:
        case hwGPIO_Int_Pin_E6:
        case hwGPIO_Int_Pin_F6:
        case hwGPIO_Int_Pin_G6:
        case hwGPIO_Int_Pin_H6:
        case hwGPIO_Int_Pin_I6:
            return 6;
            break;

        case hwGPIO_Int_Pin_A7:
        case hwGPIO_Int_Pin_B7:
        case hwGPIO_Int_Pin_C7:
        case hwGPIO_Int_Pin_D7:
        case hwGPIO_Int_Pin_E7:
        case hwGPIO_Int_Pin_F7:
        case hwGPIO_Int_Pin_G7:
        case hwGPIO_Int_Pin_H7:
        case hwGPIO_Int_Pin_I7:
            return 7;
            break;

        case hwGPIO_Int_Pin_A8:
        case hwGPIO_Int_Pin_B8:
        case hwGPIO_Int_Pin_C8:
        case hwGPIO_Int_Pin_D8:
        case hwGPIO_Int_Pin_E8:
        case hwGPIO_Int_Pin_F8:
        case hwGPIO_Int_Pin_G8:
        case hwGPIO_Int_Pin_H8:
        case hwGPIO_Int_Pin_I8:
            return 8;
            break;

        case hwGPIO_Int_Pin_A9:
        case hwGPIO_Int_Pin_B9:
        case hwGPIO_Int_Pin_C9:
        case hwGPIO_Int_Pin_D9:
        case hwGPIO_Int_Pin_E9:
        case hwGPIO_Int_Pin_F9:
        case hwGPIO_Int_Pin_G9:
        case hwGPIO_Int_Pin_H9:
        case hwGPIO_Int_Pin_I9:
            return 9;
            break;

        case hwGPIO_Int_Pin_A10:
        case hwGPIO_Int_Pin_B10:
        case hwGPIO_Int_Pin_C10:
        case hwGPIO_Int_Pin_D10:
        case hwGPIO_Int_Pin_E10:
        case hwGPIO_Int_Pin_F10:
        case hwGPIO_Int_Pin_G10:
        case hwGPIO_Int_Pin_H10:
        case hwGPIO_Int_Pin_I10:
            return 10;
            break;

        case hwGPIO_Int_Pin_A11:
        case hwGPIO_Int_Pin_B11:
        case hwGPIO_Int_Pin_C11:
        case hwGPIO_Int_Pin_D11:
        case hwGPIO_Int_Pin_E11:
        case hwGPIO_Int_Pin_F11:
        case hwGPIO_Int_Pin_G11:
        case hwGPIO_Int_Pin_H11:
        case hwGPIO_Int_Pin_I11:
            return 11;
            break;

        case hwGPIO_Int_Pin_A12:
        case hwGPIO_Int_Pin_B12:
        case hwGPIO_Int_Pin_C12:
        case hwGPIO_Int_Pin_D12:
        case hwGPIO_Int_Pin_E12:
        case hwGPIO_Int_Pin_F12:
        case hwGPIO_Int_Pin_G12:
        case hwGPIO_Int_Pin_H12:
        case hwGPIO_Int_Pin_I12:
            return 12;
            break;

        case hwGPIO_Int_Pin_A13:
        case hwGPIO_Int_Pin_B13:
        case hwGPIO_Int_Pin_C13:
        case hwGPIO_Int_Pin_D13:
        case hwGPIO_Int_Pin_E13:
        case hwGPIO_Int_Pin_F13:
        case hwGPIO_Int_Pin_G13:
        case hwGPIO_Int_Pin_H13:
        case hwGPIO_Int_Pin_I13:
            return 13;
            break;

        case hwGPIO_Int_Pin_A14:
        case hwGPIO_Int_Pin_B14:
        case hwGPIO_Int_Pin_C14:
        case hwGPIO_Int_Pin_D14:
        case hwGPIO_Int_Pin_E14:
        case hwGPIO_Int_Pin_F14:
        case hwGPIO_Int_Pin_G14:
        case hwGPIO_Int_Pin_H14:
        case hwGPIO_Int_Pin_I14:
            return 14;
            break;

        case hwGPIO_Int_Pin_A15:
        case hwGPIO_Int_Pin_B15:
        case hwGPIO_Int_Pin_C15:
        case hwGPIO_Int_Pin_D15:
        case hwGPIO_Int_Pin_E15:
        case hwGPIO_Int_Pin_F15:
        case hwGPIO_Int_Pin_G15:
        case hwGPIO_Int_Pin_H15:
        case hwGPIO_Int_Pin_I15:
            return 15;
            break;

        default:
            return 0;
            break;
    }
}

#endif // DEVICE_NUVOTON