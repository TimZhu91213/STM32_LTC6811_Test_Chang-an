# STM32_LTC6811_Test_Chang-an
> FSEC 2026 season bms 
> Develop Branch: prototype development stage

# Hardware compatibility
|1| Stm32F103C8T6 spi→ LTC6811-1 (OK)
|2| Stm32F103C8T6 spi→ LTC6820 iso-spi→ LTC6813-1 (sheduled to start in 26th June)
|3| Stm32F103C8T6 spi→ LTC6820 iso-spi→ (More than one node)LTC6813-1 (sheduled to start in 26th June)

# Respository
> Including unit test, useful code and documentation

## ChangeLog
### v0.1.0 | 2026_08_04 
- adjust wake_up delay operation

### v0.1.1 | 2026_08_10 
- attempt to integrate Simulink generation code
- update readme.md

### v0.1.2 | 2026_08_10
- use unit delay in simulink model, achieve data pass across different subsystem
- integrate FreeRtos, invoke two different taks

### v0.1.3 | 2026_08_10
- test CAN transmit __successfully__, protecting CAN operation using the FreeRTOS critical section 

### v0.1.4 | 2026_08_12
- __"F103_0811_simulink"__ project has changed irom setting, compatible with STM32 Bootloader

### v0.2.1 | 2026_08_14
- ==Transplantation== from __"F103_0811_simulink"__  TO  __"F407_VET6_Test"__ successfully, compatible with the newest bootloader project
> Realize the conditional jump from the app to the bootloader