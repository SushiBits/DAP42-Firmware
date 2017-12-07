# DAP42 Mini Debug Probe for ARM

A simple yet full featured CMSIS-DAP debug probe for ARM Cortex processors.

## Introduction

DAP42 is a CMSIS-DAP compatible USB JTAG debug adapter for ARM Cortex-A and
Cortex-M processors, based on the [STM32F042K6][42K] microcontroller.

This project is inspired by [the original DAP42 project][DAP42]. There are some
additional features implemented on top of the original project:

*   Full JTAG support. This is useful in a daisy-chained JTAG scenario.
*   When operating in SWD mode, it supports SWO trace.
*   USB MSC support with SPI Flash. This can be used to implement either built
    in driver disk, or as a production programming tool.
*   A button and an active-high control signal, allowing the device being used
    as a production programming tool.

This firmware is also used on the USB coprocessor on [SushiBits One Pro][SBONE].
The SushiBits One variant is more similar to the original DAP42, lacking the USB
MSC support, SWO code trace, and the buttons. It is based on [STM32F042F6][42F]
or [STM32F070F6][70F] microcontroller.

## The `stm32f1` branch

The `stm32f1` branch of this repository holds a version of DAP42 that supports
both USB-based CMSIS-DAP protocol and an Ethernet-based protocol, based on the
STM32F107 and eventually STM32F217 chip. This is more suitable in a more pro
environment where multiple workstations share the same test adapter and target
board.

## License and contact

This is open source hardware under [3-clause BSD license][3BSDL].

[42K]:   http://www.st.com/en/microcontrollers/stm32f042k6.html
[DAP42]: https://github.com/devanlai/dap42
[SBONE]: https://github.com/SushiBits/SushiBitsOne
[42F]:   http://www.st.com/en/microcontrollers/stm32f042f6.html
[70F]:   http://www.st.com/en/microcontrollers/stm32f070f6.html
[3BSDL]: LICENSE.md
