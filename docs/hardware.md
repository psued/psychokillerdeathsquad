# Hardware

## Main MCU

**MCU:** STM32U545VET6Q  
**Package:** LQFP100 with internal SMPS  
**Supply:** 3.3 V  
**HSE:** 16 MHz

The internal SMPS uses a 2.2 µH Vishay IHHP0805ZHER2R2M01 inductor between VLXSMPS and VDD11. The inductor and VDD11 capacitors are placed close to the MCU with a short SMPS power loop.

### Power

| Supply | Connection | Decoupling |
| --- | --- | --- |
| VDD | 3.3 V | 5 × 100 nF + 10 µF |
| VDDSMPS | 3.3 V | 10 µF |
| VDD11 | SMPS output | 2 × 2.2 µF + 2 × 100 nF |
| VDDA | 3.3 V | 100 nF + 1 µF |
| VREF+ | VDDA | 100 nF + 1 µF |
| VDDUSB | 3.3 V | 100 nF |
| VBAT | 3.3 V | 100 nF |

VSS, VSSA and VSSSMPS connect to ground.

The MCU-side 3.3 V capacity is at least 0.5 A, with approximately 0.75 A available before other shared loads. The 5 V audio rail provides approximately 1 A.

### Clock

The STM32U545 uses an Abracon ABM8W-16.0000MHZ-8-D1X-T3 16 MHz crystal with initial 2 × 10 pF C0G/NP0 load capacitors.

| Signal | MCU |
| --- | --- |
| HSE IN | PH0 |
| HSE OUT | PH1 |

No LSE is used.

## Audio

SAI1 connects to a MAX98357AETE+T Class D amplifier driving a Visaton SC 5.9 ND 4 Ω speaker.

| Signal | MCU | Amplifier | Direction |
| --- | --- | --- | --- |
| BCLK | PA8 | BCLK | MCU → amplifier |
| LRCLK | PA9 | LRCLK | MCU → amplifier |
| DATA | PA10 | DIN | MCU → amplifier |
| AUDIO_EN | PE14 | SD_MODE | MCU → amplifier |

The amplifier uses the 5 V rail and requires no MCLK. `AUDIO_EN` is low during startup. Driving it directly high enables the amplifier and selects the left I²S channel.

`GAIN_SLOT` supports all five gain configurations through resistor/link population:

| GAIN_SLOT | Gain |
| --- | ---: |
| 100 kΩ ±5% to GND | 15 dB |
| Direct to GND | 12 dB |
| Unconnected | 9 dB |
| Direct to VDD | 6 dB |
| 100 kΩ ±5% to VDD | 3 dB |

The speaker connects across the differential amplifier outputs.

Where PCB space permits, test points are included for BCLK, LRCLK, DATA, AUDIO_EN and GND.

## microSD

A Molex 503398-1892 microSD socket connects to SDMMC1 in native 4-bit mode.

| Signal | MCU |
| --- | --- |
| DAT0 | PC8 |
| DAT1 | PC9 |
| DAT2 | PC10 |
| DAT3 | PC11 |
| CLK | PC12 |
| CMD | PD2 |
| DETECT | PE15 |

CMD and DAT0–DAT3 use 47 kΩ pull-ups to 3.3 V. The card supply uses 22 µF + 100 nF decoupling. CLK has a 0 Ω series footprint. Two HSP053-4M5 devices provide ESD protection.

`SD_DETECT` uses a 10 kΩ pull-up and is active low. On the Molex 503398-1892 socket, detect pad 9 connects the switch to GND and pad 10 carries `SD_DETECT`. The switch closes when a card is inserted.

## FPGA

The FPGA interface uses 3.3 V logic.

| Signal | MCU | Direction | External bias |
| --- | --- | --- | --- |
| SCK | PB13 | MCU → FPGA | |
| MISO | PB14 | FPGA → MCU | |
| MOSI | PB15 | MCU → FPGA | |
| CS | PB0 | MCU → FPGA | pull-up |
| READY | PD3 | FPGA → MCU | 10 kΩ pull-down |
| IRQ | PD4 | FPGA → MCU | 10 kΩ pull-down |
| FRAME | PD5 | FPGA → MCU | 10 kΩ pull-down |
| AUX0 | PD6 | MCU → FPGA | |
| AUX1 | PD7 | MCU → FPGA | |

Interface behavior is documented in [`fpga.md`](fpga.md).

## Wired Controllers

Two passive controllers connect directly to STM32U545 GPIO.

| Signal | Connector pin | Controller 1 | Controller 2 |
| --- | ---: | --- | --- |
| UP | 1 | PE2 | PE8 |
| DOWN | 2 | PE3 | PE9 |
| LEFT | 3 | PE4 | PE10 |
| RIGHT | 4 | PE5 | PE11 |
| A | 5 | PE6 | PE12 |
| B | 6 | PE7 | PE13 |
| GND | 7 | | |
| GND | 8 | | |

Inputs are active low with external pull-ups and input protection, and use no MCU internal pulls. A pressed switch connects its input to ground. Exact pull-up and protection values are not yet fixed.

The controllers use Molex Micro-Fit 3.0 connectors and 8-core LAPP 28208 cable.

## Bluetooth

Three STM32WB1MMC modules are used: one console module and one in each wireless controller.

The modules use their integrated 32 MHz HSE, 32.768 kHz LSE and SMPS support. Each uses local 4.7 µF + 100 nF supply decoupling.

PB0, PB1 and PB2 are unused due to their RF sensitivity. Module placement follows the STM32WB1MMC antenna keepout requirements.

### Console Module

The console module connects to the STM32U545 through UART, reset and host interrupt signals.

| Signal | STM32U545 | STM32WB1MMC |
| --- | --- | --- |
| UART TX | PB10 / USART3_TX | PA10 / USART1_RX, pad 36 |
| UART RX | PB11 / USART3_RX | PA9 / USART1_TX, pad 38 |
| WB_NRST | PD8 | NRST, pad 23 |
| WB_HOST_IRQ | PD9 | PB8, pad 14 |

The UART baseline is 115200 baud, 8N1, without hardware flow control.

`WB_NRST` is active low. It has a 10 kΩ pull-up and is driven open drain by the STM32U545.

`WB_HOST_IRQ` is active high and held while service is required. It has a 10 kΩ pull-down.

### Wireless Controllers

Both wireless controllers use the same STM32WB1MMC pin mapping.

| Signal | GPIO | Module pad |
| --- | --- | ---: |
| UP | PA0 | 37 |
| DOWN | PA2 | 13 |
| LEFT | PA3 | 28 |
| RIGHT | PB4 | 33 |
| A | PB5 | 26 |
| B | PB6 | 27 |
| STATUS_LED | PA8 | 10 |

The six button inputs are active low with external 47 kΩ pull-ups. Pressing a button connects its input to ground.

`STATUS_LED` is active high and initially off. PA8 drives the LED through a current-limiting resistor to ground. The resistor is selected for a low indicator current suitable for battery operation.

Each wireless controller is powered directly from two AAA alkaline cells through a JS102011SCQN power switch. No regulator or charger is used.

### STM32WB Debug and Recovery

All STM32WB1MMC modules use an STDC14 connector for SWD programming and debugging.

| Signal | Module | STDC14 |
| --- | --- | --- |
| SWDIO | PA13, pad 35 | 4 |
| SWCLK | PA14, pad 34 | 6 |
| NRST | pad 23 | 12 |
| target reference | module supply | 3 |
| GND | GND | 5, 7 |

BOOT0 is pad 22, with appropriate bias and access for recovery.

## Main MCU Debug

The STM32U545 uses SWD and UART debug through an STDC14 connector.

| Signal | MCU | STDC14 |
| --- | --- | --- |
| SWDIO | PA13 | 4 |
| SWCLK | PA14 | 6 |
| NRST | NRST | 12 |
| UART4 RX | PA1 | 13 |
| UART4 TX | PA0 | 14 |
| target reference | 3.3 V | 3 |
| GND | GND | 5, 7 |

BOOT0 is PH3, with appropriate bias and access for recovery.

UART4 uses 115200 baud, 8N1. The ST-LINK exposes UART4 to the host as a Virtual COM Port through the STDC14 connection.

Test points are provided for UART4 TX, RX and GND.

## Reserved Interfaces

| Function | MCU |
| --- | --- |
| USB DM | PA11 |
| USB DP | PA12 |
| DAC1 OUT1 | PA4 |
| DAC1 OUT2 | PA5 |
