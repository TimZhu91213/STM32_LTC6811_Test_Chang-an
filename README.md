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

### v0.1.2 | 2026_0810
- use unit delay in simulink model, achieve data pass across different subsystem
- integrate FreeRtos, invoke two different taks