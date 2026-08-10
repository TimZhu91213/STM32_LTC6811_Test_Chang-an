# LTC681x 官方函数库 API 参考文档

> **工程**: F103_0804_simulink  
> **芯片型号**: LTC6813-1 (通过 `#define IC_LTC6813` 配置)  
> **兼容系列**: LTC6811-1 / LTC6813-1  
> **硬件平台**: STM32F103 (SPI1: PA5=SCK, PA6=MISO, PA7=MOSI; CS=PA4)  
> **库来源**: Analog Devices, Inc. (ADI) 官方 BMS 库, STM32/Keil 移植版  
> **生成日期**: 2026-08-09

---

## ⚠️ 重要：实际调用入口

**本工程使用 LTC6813-1 芯片，应用层代码直接调用 `LTC6813_*` 系列函数（定义在 `LTC6813_stm32.c`）。**

```
你的应用代码
    ↓ 调用
LTC6813_adcv()  LTC6813_rdcv()  LTC6813_wrcfg()  LTC6813_set_cfgr()  ...
    ↓ 内部委托给
LTC681x_adcv()  LTC681x_rdcv()  LTC681x_wrcfg()  LTC681x_set_cfgr()  ...   (LTC681x.c 核心实现)
    ↓ 调用
cmd_68()  write_68()  read_68()  wakeup_idle()  ...   (SPI 通信协议层)
    ↓ 调用
spi_write_array()  spi_write_read()  cs_low()  ...   (bms_hardware.c 硬件抽象层)
    ↓ 调用
HAL_SPI_TransmitReceive()  HAL_GPIO_WritePin()  ...   (STM32 HAL 库)
```

- **`LTC6813_*`** = 你实际调用的 API（`LTC6813_stm32.h` / `LTC6813_stm32.c`）—— **约 50 个函数**
- **`LTC681x_*`** = 底层核心实现（`LTC681x.h` / `LTC681x.c`）—— 约 65 个函数，一般**不直接调用**
- **`cmd_68 / write_68 / read_68 / wakeup_*`** = SPI 协议层—— 仅库内部使用
- **`spi_* / cs_* / delay_*`** = 硬件抽象层（`bms_hardware.c`）—— 仅库内部使用

> 本文档将 **LTC6813 API（应用层调用入口）** 放在每个分类的最前面并加粗标注，底层实现函数列在后面作为参考。

---

## 目录

1. [库文件结构](#1-库文件结构)
2. [数据结构定义](#2-数据结构定义)
3. [宏定义常量](#3-宏定义常量)
4. [底层通信与唤醒函数](#4-底层通信与唤醒函数spi-协议层库内部)
5. [PEC/CRC 校验函数](#5-peccrc-校验函数库内部)
6. [配置寄存器操作函数](#6-配置寄存器操作函数)
7. [ADC 转换启动函数](#7-adc-转换启动函数)
8. [ADC 轮询函数](#8-adc-轮询函数)
9. [数据读取与解析函数](#9-数据读取与解析函数)
10. [寄存器清除函数](#10-寄存器清除函数)
11. [自检与诊断函数](#11-自检与诊断函数)
12. [冗余与重叠测试函数](#12-冗余与重叠测试函数)
13. [开路检测函数](#13-开路检测函数)
14. [放电控制函数](#14-放电控制函数)
15. [PWM 寄存器操作函数](#15-pwm-寄存器操作函数)
16. [SCTRL 寄存器操作函数](#16-sctrl-寄存器操作函数)
17. [COMM 寄存器操作函数](#17-comm-寄存器操作函数)
18. [PEC 错误计数器管理函数](#18-pec-错误计数器管理函数)
19. [配置辅助函数 (CFGRA)](#19-配置辅助函数-cfgra)
20. [配置辅助函数 (CFGRB) — LTC6813 专有](#20-配置辅助函数-cfgrb--ltc6813-专有)
21. [LTC6813 专有命令](#21-ltc6813-专有命令)
22. [硬件抽象层函数 (HAL)](#22-硬件抽象层函数-hal内部)
23. [LTC6813 完整 API 速查表](#23-ltc6813-完整-api-速查表)
24. [ADC 模式与通道选择宏](#24-adc-模式与通道选择宏)

---

## 1. 库文件结构

| 文件 | 说明 | 层级 | 是否直接调用 |
|------|------|------|-------------|
| `LTC6813_stm32.h` / `LTC6813_stm32.c` | LTC6813-1 (18节电芯) **型号专用**封装，STM32/Keil 移植版 | **🔵 API 层（你调用的）** | ✅ **是** |
| `LTC681x.h` / `LTC681x.c` | LTC681x **通用**核心库：通信协议、寄存器读写、ADC、自检、开路检测等 | **核心实现层** | ❌ 不直接调用 |
| `bms_hardware.h` / `bms_hardware.c` | STM32 硬件抽象层：SPI 收发、CS 控制、延时 | **硬件抽象层** | ❌ 不直接调用 |
| `LTC6811.h` / `LTC6811.c` | LTC6811-1 (12节电芯) 型号专用封装（本工程不使用） | 参考 | ❌ 不使用 |

---

## 2. 数据结构定义

所有数据结构定义在 [LTC681x.h](F103_0804_simulink/LTC681x/LTC681x.h)。

| 结构体 | 用途 | 关键字段 |
|--------|------|----------|
| `cv` | 电芯电压数据 | `c_codes[18]` — 电芯电压ADC码值; `pec_match[6]` — PEC校验结果 |
| `ax` | 辅助(AUX/GPIO)电压数据 | `a_codes[9]` — AUX电压ADC码值; `pec_match[4]` — PEC校验结果 |
| `st` | 状态寄存器数据 | `stat_codes[4]` — 状态码; `flags[3]` — UV/OV标志; `mux_fail[1]` — MUX自检; `thsd[1]` — 热关断; `pec_match[2]` |
| `ic_register` | 通用IC寄存器 | `tx_data[6]` — 待发送数据; `rx_data[8]` — 接收数据; `rx_pec_match` — PEC匹配标志 |
| `pec_counter` | PEC错误计数器 | `pec_count` — 总PEC错误; `cfgr_pec` — CFGR错误; `cell_pec[6]`; `aux_pec[4]`; `stat_pec[2]` |
| `register_cfg` | 寄存器通道配置 | `cell_channels`; `stat_channels`; `aux_channels`; `num_cv_reg`; `num_gpio_reg`; `num_stat_reg` |
| `cell_asic` | **核心** IC 数据结构 | `config` / `configb` — 配置寄存器; `cells` — 电芯电压; `aux` — AUX电压; `stat` — 状态; `com` — COMM; `pwm` / `pwmb` — PWM; `sctrl` / `sctrlb` — SCTRL; `sid[6]` — 序列号; `isospi_reverse` — isoSPI方向; `crc_count` — PEC计数; `ic_reg` — 寄存器配置; `system_open_wire` — 开路检测结果 |

---

## 3. 宏定义常量

| 宏 | 值 | 说明 |
|----|-----|------|
| `MD_422HZ_1KHZ` | `0` | ADC 模式: 422Hz (正常) / 1kHz (快速) |
| `MD_27KHZ_14KHZ` | `1` | ADC 模式: 27kHz (正常) / 14kHz (快速) |
| `MD_7KHZ_3KHZ` | `2` | ADC 模式: 7kHz (正常) / 3kHz (快速) |
| `MD_26HZ_2KHZ` | `3` | ADC 模式: 26Hz (正常) / 2kHz (快速) |
| `ADC_OPT_ENABLED` | `1` | ADC 优化使能 |
| `ADC_OPT_DISABLED` | `0` | ADC 优化禁用 |
| `CELL_CH_ALL` | `0` | 转换所有电芯通道 |
| `CELL_CH_1and7` ~ `CELL_CH_6and12` | `1~6` | 转换指定电芯通道组 |
| `SELFTEST_1` / `SELFTEST_2` | `1` / `2` | 自检模式 1 或 2 |
| `AUX_CH_ALL` | `0` | 转换所有 AUX/GPIO 通道 |
| `AUX_CH_GPIO1` ~ `AUX_CH_VREF2` | `1~6` | 转换指定 AUX 通道 |
| `STAT_CH_ALL` | `0` | 转换所有状态通道 |
| `STAT_CH_SOC` / `STAT_CH_ITEMP` / `STAT_CH_VREGA` / `STAT_CH_VREGD` | `1~4` | 转换指定状态通道 |
| `REG_ALL` | `0` | 读取所有寄存器 |
| `REG_1` ~ `REG_6` | `1~6` | 读取指定寄存器 |
| `DCP_DISABLED` / `DCP_ENABLED` | `0` / `1` | 放电禁止/允许 |
| `PULL_UP_CURRENT` / `PULL_DOWN_CURRENT` | `1` / `0` | 开路检测上拉/下拉电流 |
| `CELL` / `AUX` / `STAT` / `CFGR` / `CFGRB` | `1`/`2`/`3`/`0`/`4` | 寄存器类型标识 |
| `CS_PIN` | `10` | 片选引脚号 |

---

## 4. 底层通信与唤醒函数（SPI 协议层，库内部）

> **文件**: [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **调用层级**: 仅库内部使用，应用代码**不直接调用**  
> **说明**: isoSPI 总线底层通信函数，所有 LTC68xx 操作的基础。

### 4.1 `wakeup_idle()`

| 属性 | 内容 |
|------|------|
| **声明** | `void wakeup_idle(uint8_t total_ic)` |
| **功能** | 将 isoSPI 从 IDLE 状态唤醒，进入 READY 状态 |
| **参数** | `total_ic` — 菊花链中 IC 总数量 |
| **返回值** | 无 |
| **实现** | 每个 IC 发送 1 字节 `0xFF`，确保 isoSPI 进入就绪模式 |

### 4.2 `wakeup_sleep()`

| 属性 | 内容 |
|------|------|
| **声明** | `void wakeup_sleep(uint8_t total_ic)` |
| **功能** | 将 LTC681x 从 SLEEP 状态唤醒 |
| **参数** | `total_ic` — 菊花链中 IC 总数量 |
| **返回值** | 无 |
| **实现** | 每个 IC 拉低 CS ≥300μs，确保芯片进入待机模式 |

### 4.3 `cmd_68()`

| 属性 | 内容 |
|------|------|
| **声明** | `void cmd_68(uint8_t tx_cmd[2])` |
| **功能** | 向 BMS IC 发送 2 字节命令（自动计算并附加 PEC） |
| **参数** | `tx_cmd[2]` — 2 字节命令数组 |
| **返回值** | 无 |
| **实现** | 计算 PEC，组装 4 字节 (cmd[2] + PEC[2])，通过 SPI 发送 |

### 4.4 `write_68()`

| 属性 | 内容 |
|------|------|
| **声明** | `void write_68(uint8_t total_ic, uint8_t tx_cmd[2], uint8_t data[])` |
| **功能** | 向菊花链写入命令和数据（自动计算命令和数据两级 PEC） |
| **参数** | `total_ic` — IC 数量; `tx_cmd[2]` — 命令; `data[]` — 待写入数据(每个 IC 6 字节 + 2 字节 PEC) |
| **返回值** | 无 |
| **注意** | 数据按降序写入（最后一个 IC 先收到配置） |

### 4.5 `read_68()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t read_68(uint8_t total_ic, uint8_t tx_cmd[2], uint8_t *rx_data)` |
| **功能** | 发送命令并从菊花链读取数据（每个 IC 8 字节），自动校验 PEC |
| **参数** | `total_ic` — IC 数量; `tx_cmd[2]` — 命令; `rx_data` — 接收缓冲区 |
| **返回值** | `0` — PEC 匹配成功; `-1` — 存在 PEC 错误 |

---

## 5. PEC/CRC 校验函数（库内部）

> **文件**: [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **调用层级**: 仅库内部使用，应用代码**不直接调用**

### 5.1 `pec15_calc()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint16_t pec15_calc(uint8_t len, uint8_t *data)` |
| **功能** | 计算 15 位 CRC (PEC15)，用于所有 LTC68xx 通信的差错校验 |
| **参数** | `len` — 数据长度(字节); `data` — 数据数组 |
| **返回值** | 计算得到的 16 位 PEC 值 (LSB 为 0) |
| **实现** | 基于预计算的 256 项 CRC15 查找表 `crc15Table[256]` |

### 5.2 `parse_cells()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t parse_cells(uint8_t current_ic, uint8_t cell_reg, uint8_t cell_data[], uint16_t *cell_codes, uint8_t *ic_pec)` |
| **功能** | 解析电压测量寄存器数据（通用解析器，同时用于 CELL/AUX 寄存器） |
| **参数** | `current_ic` — 当前 IC 索引; `cell_reg` — 寄存器类型; `cell_data[]` — 原始数据; `cell_codes` — 解析后的电压码; `ic_pec` — PEC 错误标志 |
| **返回值** | `0` — 无 PEC 错误; `1` — 检测到 PEC 错误 |

---

## 6. 配置寄存器操作函数

> **🔵 应用层调用**: `LTC6813_wrcfg()` `LTC6813_wrcfgb()` `LTC6813_rdcfg()` `LTC6813_rdcfgb()`  
> **底层实现**: `LTC681x_wrcfg()` `LTC681x_wrcfgb()` `LTC681x_rdcfg()` `LTC681x_rdcfgb()` — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **说明**: LTC6813 的 4 个配置寄存器操作函数都是对 LTC681x 对应函数的**薄封装**。

### 🔵 API: `LTC6813_wrcfg()` → 内部调用 `LTC681x_wrcfg()`
### 🔵 API: `LTC6813_wrcfgb()` → 内部调用 `LTC681x_wrcfgb()`
### 🔵 API: `LTC6813_rdcfg()` → 内部调用 `LTC681x_rdcfg()`
### 🔵 API: `LTC6813_rdcfgb()` → 内部调用 `LTC681x_rdcfgb()`

### 6.1 `LTC681x_wrcfg()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_wrcfg(uint8_t total_ic, cell_asic *ic)` |
| **功能** | 写入 CFGRA 配置寄存器（菊花链降序写入） |
| **命令码** | `0x0001` (WRCFGA) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | 无 |

### 6.2 `LTC681x_wrcfgb()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_wrcfgb(uint8_t total_ic, cell_asic *ic)` |
| **功能** | 写入 CFGRB 配置寄存器 |
| **命令码** | `0x0024` (WRCFGB) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | 无 |

### 6.3 `LTC681x_rdcfg()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdcfg(uint8_t total_ic, cell_asic *ic)` |
| **功能** | 读取 CFGRA 配置寄存器 |
| **命令码** | `0x0002` (RDCFGA) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 (存储读取数据) |
| **返回值** | `0` — PEC 匹配; `-1` — PEC 错误 |

### 6.4 `LTC681x_rdcfgb()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdcfgb(uint8_t total_ic, cell_asic *ic)` |
| **功能** | 读取 CFGRB 配置寄存器 |
| **命令码** | `0x0026` (RDCFGB) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 (存储读取数据) |
| **返回值** | `0` — PEC 匹配; `-1` — PEC 错误 |

---

## 7. ADC 转换启动函数

> **🔵 应用层调用**: `LTC6813_adcv()` `LTC6813_adax()` `LTC6813_adstat()` `LTC6813_adcvax()` `LTC6813_adcvsc()`  
> **底层实现**: `LTC681x_adcv()` 等 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **说明**: 5 个 LTC6813 ADC 启动函数都是直接委托给 LTC681x。

### 7.1 `LTC681x_adcv()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adcv(uint8_t MD, uint8_t DCP, uint8_t CH)` |
| **功能** | **启动电芯电压 ADC 转换** — 测量所有或选定电芯通道的电压 |
| **参数** | `MD` — ADC 转换模式 (4种: 422Hz/27kHz/7kHz/26Hz); `DCP` — 转换期间是否允许放电; `CH` — 电芯通道选择 (`CELL_CH_ALL` 等) |
| **命令码** | `0x0260` + MD + DCP + CH (ADCV) |

### 7.2 `LTC681x_adax()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adax(uint8_t MD, uint8_t CHG)` |
| **功能** | **启动 GPIO 和 Vref2 ADC 转换** — 测量 GPIO 引脚电压和第二参考电压 |
| **参数** | `MD` — ADC 模式; `CHG` — GPIO 通道选择 (`AUX_CH_ALL`, `AUX_CH_GPIO1`~`AUX_CH_VREF2`) |
| **命令码** | `0x0460` + MD + CHG (ADAX) |

### 7.3 `LTC681x_adstat()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adstat(uint8_t MD, uint8_t CHST)` |
| **功能** | **启动状态 ADC 转换** — 测量 SOC/内部温度/VREGA/VREGD 等状态量 |
| **参数** | `MD` — ADC 模式; `CHST` — 状态通道选择 (`STAT_CH_ALL`, `STAT_CH_SOC` 等) |
| **命令码** | `0x0468` + MD + CHST (ADSTAT) |

### 7.4 `LTC681x_adcvax()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adcvax(uint8_t MD, uint8_t DCP)` |
| **功能** | **启动电芯电压 + GPIO1&2 联合转换** — 同时测量所有电芯和 GPIO1/GPIO2 |
| **参数** | `MD` — ADC 模式; `DCP` — 放电允许 |
| **命令码** | `0x046F` + MD + DCP (ADCVAC) |

### 7.5 `LTC681x_adcvsc()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adcvsc(uint8_t MD, uint8_t DCP)` |
| **功能** | **启动电芯电压 + SOC 联合转换** — 同时测量所有电芯和电芯之和 (Sum of Cells) |
| **参数** | `MD` — ADC 模式; `DCP` — 放电允许 |
| **命令码** | `0x0467` + MD + DCP (ADCVSC) |

---

## 8. ADC 轮询函数

> **🔵 应用层调用**: `LTC6813_pladc()` `LTC6813_pollAdc()`  
> **底层实现**: `LTC681x_pladc()` `LTC681x_pollAdc()` — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 8.1 `LTC681x_pladc()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint8_t LTC681x_pladc()` |
| **功能** | **发送 ADC 轮询命令** — 查询 ADC 转换是否完成 |
| **命令码** | `0x0714` (PLADC) |
| **返回值** | `0xFF` — 转换未完成; 非 `0xFF` — 转换已完成 |
| **注意** | 非阻塞，仅查询一次 |

### 8.2 `LTC681x_pollAdc()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint32_t LTC681x_pollAdc()` |
| **功能** | **阻塞等待 ADC 转换完成** — 持续轮询直到转换完毕或超时 |
| **返回值** | 近似等待时间计数器值（最大 200,000 次轮询） |

---

## 9. 数据读取与解析函数

> **🔵 应用层调用**: `LTC6813_rdcv()` `LTC6813_rdaux()` `LTC6813_rdstat()`  
> **底层实现**: `LTC681x_rdcv()` `LTC681x_rdaux()` `LTC681x_rdstat()` 等 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **说明**: 3 个 LTC6813 读取函数直接委托给 LTC681x。`_reg` 后缀的原始读取函数仅库内部使用。

### 9.1 `LTC681x_rdcv()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint8_t LTC681x_rdcv(uint8_t reg, uint8_t total_ic, cell_asic *ic)` |
| **功能** | **读取并解析电芯电压寄存器** — 读取所有或指定电芯电压寄存器，解析后存入 `ic[].cells.c_codes[]` |
| **参数** | `reg` — 寄存器选择 (`REG_ALL`=0 读取全部; `REG_1`~`REG_6` 读单个); `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | `0` — 无 PEC 错误; `-1` — 检测到 PEC 错误 |

### 9.2 `LTC681x_rdaux()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdaux(uint8_t reg, uint8_t total_ic, cell_asic *ic)` |
| **功能** | **读取并解析 AUX/GPIO 寄存器** — 解析后存入 `ic[].aux.a_codes[]` |
| **参数** | `reg` — 寄存器选择; `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | `0` — 无 PEC 错误; `-1` — PEC 错误 |

### 9.3 `LTC681x_rdstat()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdstat(uint8_t reg, uint8_t total_ic, cell_asic *ic)` |
| **功能** | **读取并解析状态寄存器** — 解析 SOC/温度/VREG 状态值、UV/OV 标志、MUX 状态、热关断状态 |
| **参数** | `reg` — 寄存器选择 (1=STAT-A, 2=STAT-B); `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | `0` — 无 PEC 错误; `-1` — PEC 错误 |

### 9.4 `LTC681x_rdcv_reg()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_rdcv_reg(uint8_t reg, uint8_t total_ic, uint8_t *data)` |
| **功能** | **读取原始电芯电压寄存器数据** (不解析) — 底层函数，通常由 `LTC681x_rdcv()` 调用 |
| **参数** | `reg` — 寄存器选择 (1=RDCVA, 2=RDCVB, ..., 6=RDCVF); `total_ic` — IC 数量; `data` — 原始数据缓冲区 |

### 9.5 `LTC681x_rdaux_reg()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_rdaux_reg(uint8_t reg, uint8_t total_ic, uint8_t *data)` |
| **功能** | **读取原始 AUX 寄存器数据** (不解析) — 底层函数，通常由 `LTC681x_rdaux()` 调用 |
| **参数** | `reg` — 寄存器选择 (1=RDAUXA, 2=RDAUXB, 3=RDAUXC, 4=RDAUXD) |

### 9.6 `LTC681x_rdstat_reg()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_rdstat_reg(uint8_t reg, uint8_t total_ic, uint8_t *data)` |
| **功能** | **读取原始状态寄存器数据** (不解析) — 底层函数，通常由 `LTC681x_rdstat()` 调用 |
| **参数** | `reg` — 寄存器选择 (1=STAT-A, 2=STAT-B) |

---

## 10. 寄存器清除函数

> **🔵 应用层调用**: `LTC6813_clrcell()` `LTC6813_clraux()` `LTC6813_clrstat()` `LTC6813_clrsctrl()`  
> **底层实现**: `LTC681x_clrcell()` 等 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 10.1 `LTC681x_clrcell()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_clrcell()` |
| **功能** | **清除电芯电压寄存器** — 所有值初始化为 1 (0xFF) |
| **命令码** | `0x0711` (CLRCELL) |

### 10.2 `LTC681x_clraux()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_clraux()` |
| **功能** | **清除辅助(AUX)寄存器** — 所有值初始化为 1 |
| **命令码** | `0x0712` (CLRAUX) |

### 10.3 `LTC681x_clrstat()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_clrstat()` |
| **功能** | **清除状态寄存器** — 所有值初始化为 1 |
| **命令码** | `0x0713` (CLRSTAT) |

### 10.4 `LTC681x_clrsctrl()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_clrsctrl()` |
| **功能** | **清除 SCTRL 寄存器** — 所有值初始化为 0 (区别于其他清除函数) |
| **命令码** | `0x0018` (CLRSCTRL) |

---

## 11. 自检与诊断函数

> **🔵 应用层调用**: `LTC6813_diagn()` `LTC6813_cvst()` `LTC6813_axst()` `LTC6813_statst()`  
> **底层实现**: `LTC681x_diagn()` 等 + `LTC681x_st_lookup()` `LTC681x_run_cell_adc_st()` — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 11.1 `LTC681x_diagn()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_diagn()` |
| **功能** | **启动 Mux 解码器诊断自检** — 约 1ms 完成，结果通过 MUXFAIL 位反映 |
| **命令码** | `0x0715` (DIAGN) |

### 11.2 `LTC681x_cvst()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_cvst(uint8_t MD, uint8_t ST)` |
| **功能** | **启动电芯电压自检转换** — 测量已知内部测试信号验证 ADC 精度 |
| **参数** | `MD` — ADC 模式; `ST` — 自检选择 (`SELFTEST_1` 或 `SELFTEST_2`) |

### 11.3 `LTC681x_axst()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_axst(uint8_t MD, uint8_t ST)` |
| **功能** | **启动辅助(AUX)寄存器自检转换** |
| **参数** | `MD` — ADC 模式; `ST` — 自检选择 |

### 11.4 `LTC681x_statst()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_statst(uint8_t MD, uint8_t ST)` |
| **功能** | **启动状态寄存器自检转换** |
| **参数** | `MD` — ADC 模式; `ST` — 自检选择 |

### 11.5 `LTC681x_st_lookup()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint16_t LTC681x_st_lookup(uint8_t MD, uint8_t ST, bool adcopt)` |
| **功能** | **自检期望值查表** — 返回给定 ADC 模式和自检类型的期望数据模式 |
| **参数** | `MD` — ADC 模式; `ST` — 自检选择; `adcopt` — ADCOPT 位 |
| **返回值** | 16 位期望值（如 `0x9565`, `0x6A9A`, `0x9555`, `0x6AAA` 等） |

---

## 12. 冗余与重叠测试函数

> **🔵 应用层调用**: `LTC6813_adol()` `LTC6813_adaxd()` `LTC6813_adstatd()` `LTC6813_run_cell_adc_st()` `LTC6813_run_adc_overlap()` `LTC6813_run_adc_redundancy_st()`  
> **底层实现**: 对应 `LTC681x_*` 函数 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)  
> **注意**: `LTC6813_run_adc_overlap()` 有独立实现（18 节电芯多检测一对通道），其余为薄封装。

### 12.1 `LTC681x_adol()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adol(uint8_t MD, uint8_t DCP)` |
| **功能** | **启动电芯电压重叠(Overlap)转换** — 用于检测 ADC 测量精度 |
| **参数** | `MD` — ADC 模式; `DCP` — 放电允许 |

### 12.2 `LTC681x_adaxd()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adaxd(uint8_t MD, uint8_t CHG)` |
| **功能** | **启动 GPIO 冗余测试** — 通过数字比较器验证 GPIO 测量 |
| **参数** | `MD` — ADC 模式; `CHG` — GPIO 通道选择 |

### 12.3 `LTC681x_adstatd()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adstatd(uint8_t MD, uint8_t CHST)` |
| **功能** | **启动状态寄存器冗余测试** |
| **参数** | `MD` — ADC 模式; `CHST` — 状态通道选择 |

### 12.4 `LTC681x_run_cell_adc_st()`

| 属性 | 内容 |
|------|------|
| **声明** | `int16_t LTC681x_run_cell_adc_st(uint8_t adc_reg, uint8_t total_ic, cell_asic *ic, uint8_t md, bool adcopt)` |
| **功能** | **运行 ADC 数字滤波器自检** — 对 CELL/AUX/STAT 寄存器执行完整自检流程，比较测量值与期望值 |
| **参数** | `adc_reg` — 寄存器类型 (`CELL`/`AUX`/`STAT`); `total_ic` — IC 数量; `ic` — 数据存储; `md` — ADC 模式; `adcopt` — ADCOPT 配置 |
| **返回值** | 错误数量（0 = 通过） |

### 12.5 `LTC681x_run_adc_overlap()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint16_t LTC681x_run_adc_overlap(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **运行 ADC 重叠测试** — 比较相邻通道测量差距 (6-7通道)，阈值 ±20 |
| **参数** | `total_ic` — IC 数量; `ic` — 数据存储 |
| **返回值** | `0` — 通过; 非零 — 对应 IC 位被置位表示失败 |

### 12.6 `LTC681x_run_adc_redundancy_st()`

| 属性 | 内容 |
|------|------|
| **声明** | `int16_t LTC681x_run_adc_redundancy_st(uint8_t adc_mode, uint8_t adc_reg, uint8_t total_ic, cell_asic *ic)` |
| **功能** | **运行 ADC 数字冗余自检** — 检查 AUX/STAT 测量值是否 ≥65280（溢出） |
| **参数** | `adc_mode` — ADC 模式; `adc_reg` — 寄存器类型 (`AUX`/`STAT`); `total_ic` — IC 数量; `ic` — 数据存储 |
| **返回值** | 错误数量（0 = 通过） |

---

## 13. 开路检测函数

> **🔵 应用层调用**: `LTC6813_adow()` `LTC6813_axow()` `LTC6813_run_openwire_single()` `LTC6813_run_openwire_multi()` `LTC6813_run_gpio_openwire()`  
> **底层实现**: 对应 `LTC681x_*` 函数 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 13.1 `LTC681x_adow()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_adow(uint8_t MD, uint8_t PUP, uint8_t CH, uint8_t DCP)` |
| **功能** | **启动电芯开路检测 ADC 转换** — 施加 Pull-Up/Pull-Down 电流后测量 |
| **参数** | `MD` — ADC 模式; `PUP` — `PULL_UP_CURRENT`(1) 或 `PULL_DOWN_CURRENT`(0); `CH` — 通道; `DCP` — 放电允许 |

### 13.2 `LTC681x_axow()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_axow(uint8_t MD, uint8_t PUP)` |
| **功能** | **启动 GPIO 开路检测 ADC 转换** |
| **参数** | `MD` — ADC 模式; `PUP` — 上拉/下拉选择 |

### 13.3 `LTC681x_run_openwire_single()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_run_openwire_single(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **运行单节电芯开路检测** — 按数据手册算法：3 次上拉 + 3 次下拉测量，比较差值 >4000 即判定开路 |
| **参数** | `total_ic` — IC 数量; `ic` — 存储数据，结果写入 `ic[].system_open_wire` |

### 13.4 `LTC681x_run_openwire_multi()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_run_openwire_multi(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **运行多节/连续两节电芯开路检测** — 5次上拉 + 5次下拉，更全面的检测算法 |
| **参数** | `total_ic` — IC 数量; `ic` — 存储数据 |

### 13.5 `LTC681x_run_gpio_openwire()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_run_gpio_openwire(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **运行 GPIO 开路检测** — 阈值 150，比较正常测量与下拉测量的差值 |
| **参数** | `total_ic` — IC 数量; `ic` — 存储数据，结果写入 `ic[].system_open_wire` |

---

## 14. 放电控制函数

> **🔵 应用层调用**: `LTC6813_set_discharge()` `LTC6813_clear_discharge()`  
> **底层实现**: `LTC681x_clear_discharge()` 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c); `LTC6813_set_discharge()` 有独立实现（18 节电芯覆盖 CFGRA+CFGRB）

### 14.1 `LTC681x_clear_discharge()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_clear_discharge(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **清除所有 DCC (放电控制) 位** — 将 CFGRA 和 CFGRB 中的放电位全部清零 |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |

---

## 15. PWM 寄存器操作函数

> **🔵 应用层调用**: `LTC6813_wrpwm()` `LTC6813_rdpwm()`  
> **底层实现**: `LTC681x_wrpwm()` `LTC681x_rdpwm()` — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 15.1 `LTC681x_wrpwm()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_wrpwm(uint8_t total_ic, uint8_t pwmReg, cell_asic *ic)` |
| **功能** | **写入 PWM 寄存器** (A 或 B) |
| **参数** | `total_ic` — IC 数量; `pwmReg` — `0`=PWM-A (0x0020), 非0=PWM-B (0x001C); `ic` — 数据 |
| **命令码** | PWM-A: `0x0020` (WRPWMA); PWM-B: `0x001C` (WRPWMB) |

### 15.2 `LTC681x_rdpwm()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdpwm(uint8_t total_ic, uint8_t pwmReg, cell_asic *ic)` |
| **功能** | **读取 PWM 寄存器** (A 或 B) |
| **参数** | `total_ic` — IC 数量; `pwmReg` — `0`=PWM-A (0x0022), 非0=PWM-B (0x001E); `ic` — 存储读取数据 |
| **返回值** | `0` — PEC 匹配; `-1` — PEC 错误 |

---

## 16. SCTRL 寄存器操作函数

> **🔵 应用层调用**: `LTC6813_wrsctrl()` `LTC6813_rdsctrl()` `LTC6813_stsctrl()` `LTC6813_clrsctrl()`  
> **底层实现**: `LTC681x_wrsctrl()` 等 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 16.1 `LTC681x_wrsctrl()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_wrsctrl(uint8_t total_ic, uint8_t sctrl_reg, cell_asic *ic)` |
| **功能** | **写入 SCTRL 寄存器** (A 或 B) — 用于控制 S 引脚通信 |
| **参数** | `total_ic` — IC 数量; `sctrl_reg` — `0`=SCTRL-A (0x0014), 非0=SCTRL-B (0x001C); `ic` — 数据 |
| **命令码** | SCTRL-A: `0x0014` (WRSCTRLA); SCTRL-B: `0x001C` (WRSCTRLB) |

### 16.2 `LTC681x_rdsctrl()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdsctrl(uint8_t total_ic, uint8_t sctrl_reg, cell_asic *ic)` |
| **功能** | **读取 SCTRL 寄存器** (A 或 B) |
| **参数** | `total_ic` — IC 数量; `sctrl_reg` — `0`=SCTRL-A (0x0016), 非0=SCTRL-B (0x001E) |
| **返回值** | `0` — PEC 匹配; `-1` — PEC 错误 |

### 16.3 `LTC681x_stsctrl()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_stsctrl()` |
| **功能** | **启动 SCTRL 数据通信** — 通过 S 引脚发送脉冲通信数据 |
| **命令码** | `0x0019` (STSCTRL) |

---

## 17. COMM 寄存器操作函数

> **🔵 应用层调用**: `LTC6813_wrcomm()` `LTC6813_rdcomm()` `LTC6813_stcomm()`  
> **底层实现**: `LTC681x_wrcomm()` 等 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 17.1 `LTC681x_wrcomm()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_wrcomm(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **写入 COMM 寄存器** — 用于菊花链 I2C/SPI 桥接通信 |
| **命令码** | `0x0721` (WRCOMM) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |

### 17.2 `LTC681x_rdcomm()`

| 属性 | 内容 |
|------|------|
| **声明** | `int8_t LTC681x_rdcomm(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **读取 COMM 寄存器** |
| **命令码** | `0x0722` (RDCOMM) |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |
| **返回值** | `0` — PEC 匹配; `-1` — PEC 错误 |

### 17.3 `LTC681x_stcomm()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_stcomm(uint8_t len)` |
| **功能** | **启动 COMM 数据输出** — 将 COMM 寄存器中的数据通过 IC 的 I2C/SPI 端口输出 |
| **命令码** | `0x0723` (STCOMM) |
| **参数** | `len` — 要传输的数据长度 (字节) |

---

## 18. PEC 错误计数器管理函数

> **🔵 应用层调用**: `LTC6813_check_pec()` `LTC6813_reset_crc_count()`  
> **底层实现**: `LTC681x_check_pec()` `LTC681x_reset_crc_count()` — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 18.1 `LTC681x_check_pec()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_check_pec(uint8_t total_ic, uint8_t reg, cell_asic *ic)` |
| **功能** | **累加 PEC 错误计数** — 根据寄存器类型递增 `pec_counter` 结构体中对应计数器 |
| **参数** | `total_ic` — IC 数量; `reg` — 寄存器类型 (`CFGR`/`CFGRB`/`CELL`/`AUX`/`STAT`); `ic` — 数据 |

### 18.2 `LTC681x_reset_crc_count()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_reset_crc_count(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **重置所有 PEC 错误计数器** — 将 `pec_count`、`cfgr_pec`、`cell_pec[]`、`aux_pec[]`、`stat_pec[]` 全部清零 |
| **参数** | `total_ic` — IC 数量; `ic` — 数据 |

---

## 19. 配置辅助函数 (CFGRA)

> **🔵 应用层调用**: `LTC6813_init_cfg()` `LTC6813_set_cfgr()` `LTC6813_set_cfgr_refon()` `LTC6813_set_cfgr_adcopt()` `LTC6813_set_cfgr_gpio()` `LTC6813_set_cfgr_dis()` `LTC6813_set_cfgr_uv()` `LTC6813_set_cfgr_dcto()` `LTC6813_set_cfgr_ov()`  
> **底层实现**: 对应 `LTC681x_*` 函数 — 在 [LTC681x.c](F103_0804_simulink/LTC681x/LTC681x.c)

### 19.1 `LTC681x_init_cfg()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_init_cfg(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **初始化所有 IC 的 CFGRA 数据结构** — 全部清零 |
| **参数** | `total_ic` — IC 数量; `ic` — cell_asic 数组 |

### 19.2 `LTC681x_set_cfgr()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr(uint8_t nIC, cell_asic *ic, bool refon, bool adcopt, bool gpio[5], bool dcc[12], bool dcto[4], uint16_t uv, uint16_t ov)` |
| **功能** | **综合设置 CFGRA 所有字段** — 一次性配置 REFON/ADCOPT/GPIO/DCC/DCTO/UV/OV |
| **参数** | `nIC` — IC 索引; `refon` — 参考电压使能; `adcopt` — ADC 优化; `gpio[5]` — 5 个 GPIO 引脚方向; `dcc[12]` — 12 路放电控制; `dcto[4]` — 放电超时; `uv` — 欠压阈值; `ov` — 过压阈值 |

### 19.3 `LTC681x_set_cfgr_refon()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_refon(uint8_t nIC, cell_asic *ic, bool refon)` |
| **功能** | **设置 REFON 位** (bit 2 of CFGRA[0]) — 控制内部 3V 参考电压 |
| **参数** | `nIC` — IC 索引; `refon` — `true`=使能, `false`=关闭 |

### 19.4 `LTC681x_set_cfgr_adcopt()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_adcopt(uint8_t nIC, cell_asic *ic, bool adcopt)` |
| **功能** | **设置 ADCOPT 位** (bit 0 of CFGRA[0]) — 控制 ADC 工作模式 |
| **参数** | `nIC` — IC 索引; `adcopt` — `true`/`false` |

### 19.5 `LTC681x_set_cfgr_gpio()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_gpio(uint8_t nIC, cell_asic *ic, bool gpio[])` |
| **功能** | **设置 GPIO[1:5] 引脚方向** (bits 3-7 of CFGRA[0]) — `true`=输出, `false`=输入 |
| **参数** | `nIC` — IC 索引; `gpio[5]` — 5 个 GPIO 引脚方向 |

### 19.6 `LTC681x_set_cfgr_dis()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_dis(uint8_t nIC, cell_asic *ic, bool dcc[])` |
| **功能** | **设置 DCC[1:12] 放电控制位** — `true`=放电使能, `false`=禁止 |
| **参数** | `nIC` — IC 索引; `dcc[12]` — 12 路放电控制 (低8位在 CFGRA[4], 高4位在 CFGRA[5] bit 0-3) |

### 19.7 `LTC681x_set_cfgr_dcto()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_dcto(uint8_t nIC, cell_asic *ic, bool dcto[])` |
| **功能** | **设置 DCTO[0:3] 放电超时** (CFGRA[5] bits 4-7) |
| **参数** | `nIC` — IC 索引; `dcto[4]` — 放电超时设置 (0000=禁用, 0001=0.5min, ..., 1111=256min) |

### 19.8 `LTC681x_set_cfgr_uv()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_uv(uint8_t nIC, cell_asic *ic, uint16_t uv)` |
| **功能** | **设置欠压(UV)阈值** — 写入 CFGRA[1:2] 相关字段 |
| **参数** | `nIC` — IC 索引; `uv` — 欠压阈值 (16mV 步长，公式: tmp = uv/16 - 1) |

### 19.9 `LTC681x_set_cfgr_ov()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC681x_set_cfgr_ov(uint8_t nIC, cell_asic *ic, uint16_t ov)` |
| **功能** | **设置过压(OV)阈值** — 写入 CFGRA[2:3] 相关字段 |
| **参数** | `nIC` — IC 索引; `ov` — 过压阈值 (16mV 步长，公式: tmp = ov/16) |

---

## 20. 配置辅助函数 (CFGRB) — LTC6813 专有

> **🔵 应用层调用（LTC6813 独有）**: `LTC6813_init_cfgb()` `LTC6813_set_cfgrb()` `LTC6813_set_cfgrb_fdrf()` `LTC6813_set_cfgrb_dtmen()` `LTC6813_set_cfgrb_ps()` `LTC6813_set_cfgrb_gpio_b()` `LTC6813_set_cfgrb_dcc_b()`  
> **文件**: [LTC6813_stm32.c](F103_0804_simulink/LTC681x/LTC6813_stm32.c) — 这些函数**仅在 LTC6813 上存在**，LTC6811 没有 CFGRB  
> **说明**: LTC6813 相比 LTC6811 多一个 CFGRB 配置寄存器，用于控制额外的 6 路电芯和 GPIO。

### 20.1 `LTC6813_init_cfgb()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_init_cfgb(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **初始化所有 IC 的 CFGRB 数据结构** — 全部清零 |

### 20.2 `LTC6813_set_cfgrb()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb(uint8_t nIC, cell_asic *ic, bool fdrf, bool dtmen, bool ps[2], bool gpiobits[4], bool dccbits[7])` |
| **功能** | **综合设置 CFGRB 所有字段** |
| **参数** | `fdrf` — 快速放电参考; `dtmen` — 放电定时器使能; `ps[2]` — S 引脚路径选择; `gpiobits[4]` — GPIO6-9 方向; `dccbits[7]` — DCC 13-18 + DCC0 放电控制 |

### 20.3 `LTC6813_set_cfgrb_fdrf()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb_fdrf(uint8_t nIC, cell_asic *ic, bool fdrf)` |
| **功能** | **设置 FDRF 位** (CFGRB[1] bit 6) — 快速放电参考使能 |

### 20.4 `LTC6813_set_cfgrb_dtmen()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb_dtmen(uint8_t nIC, cell_asic *ic, bool dtmen)` |
| **功能** | **设置 DTMEN 位** (CFGRB[1] bit 3) — 放电定时器使能 |

### 20.5 `LTC6813_set_cfgrb_ps()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb_ps(uint8_t nIC, cell_asic *ic, bool ps[])` |
| **功能** | **设置 PS[1:2] S 引脚路径选择** (CFGRB[1] bits 4-5) |

### 20.6 `LTC6813_set_cfgrb_gpio_b()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb_gpio_b(uint8_t nIC, cell_asic *ic, bool gpiobits[])` |
| **功能** | **设置 CFGRB 中 GPIO6-9 方向** (CFGRB[0] bits 0-3) |

### 20.7 `LTC6813_set_cfgrb_dcc_b()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_cfgrb_dcc_b(uint8_t nIC, cell_asic *ic, bool dccbits[])` |
| **功能** | **设置 CFGRB 中 DCC 扩展位** — DCC0 (CFGRB[1] bit 2) + DCC13-18 (CFGRB[0] bits 4-7, CFGRB[1] bits 0-1) |

---

## 21. LTC6813 专有命令

> **🔵 应用层调用（LTC6813 独有，LTC6811 不存在）**: `LTC6813_init_reg_limits()` `LTC6813_mute()` `LTC6813_unmute()` `LTC6813_wrpsb()` `LTC6813_rdpsb()`  
> **文件**: [LTC6813_stm32.c](F103_0804_simulink/LTC681x/LTC6813_stm32.c)  
> **说明**: LTC6813-1 (18节电芯) 专有的命令和功能，LTC6811 不支持。

### 21.1 `LTC6813_init_reg_limits()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_init_reg_limits(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **初始化 LTC6813 寄存器通道限制** — 18 电芯 / 4 状态 / 9 AUX / 6 CV 寄存器 / 4 GPIO 寄存器 / 2 状态寄存器 |

### 21.2 `LTC6813_set_discharge()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_set_discharge(int Cell, uint8_t total_ic, cell_asic *ic)` |
| **功能** | **设置单个电芯放电** — Cell=0 对应 DCC0 (CFGRB), Cell=1-8→CFGRA[4], Cell=9-12→CFGRA[5], Cell=13-16→CFGRB[0], Cell=17-18→CFGRB[1] |

### 21.3 `LTC6813_clear_discharge()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_clear_discharge(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **清除所有放电位** — 封装 `LTC681x_clear_discharge()` |

### 21.4 `LTC6813_mute()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_mute()` |
| **功能** | **静音(禁止)放电晶体管** — LTC6813 专有命令 |
| **命令码** | `0x0028` (MUTE) |

### 21.5 `LTC6813_unmute()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_unmute()` |
| **功能** | **解除放电晶体管静音** |
| **命令码** | `0x0029` (UNMUTE) |

### 21.6 `LTC6813_wrpsb()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6813_wrpsb(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **写入 PWM/SCTRL 寄存器 B 组合** — LTC6813 将 PWMB 和 SCTRLB 合并为一个 6 字节寄存器 |
| **命令码** | `0x001C` (WRPSB) |

### 21.7 `LTC6813_rdpsb()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint8_t LTC6813_rdpsb(uint8_t total_ic, cell_asic *ic)` |
| **功能** | **读取 PWM/SCTRL 寄存器 B 组合** |
| **命令码** | `0x001E` (RDPSB) |
| **返回值** | PEC 状态 (`0`=匹配, `-1`=错误) |

---

## 22. LTC6811 特有功能函数（本工程不使用，仅供参考）

> **⚠️ 本工程使用 LTC6813，不调用这些函数。**  
> **文件**: [LTC6811.c](F103_0804_simulink/LTC681x/LTC6811.c)  
> **说明**: LTC6811-1 (12节电芯) 专有的功能函数。

### 22.1 `LTC6811_init_reg_limits()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6811_init_reg_limits(uint8_t total_ic, cell_asic ic[])` |
| **功能** | **初始化 LTC6811 寄存器通道限制** — 12 电芯 / 4 状态 / 6 AUX / 4 CV 寄存器 / 2 GPIO 寄存器 / 3 状态寄存器 |

### 22.2 `LTC6811_set_discharge()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6811_set_discharge(int Cell, uint8_t total_ic, cell_asic ic[])` |
| **功能** | **设置单个电芯放电** — Cell 1-8→CFGRA[4], Cell 9-12→CFGRA[5] |

### 22.3 `LTC6811_init_max_min()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6811_init_max_min(uint8_t total_ic, cell_asic ic[], cell_asic ic_max[], cell_asic ic_min[])` |
| **功能** | **初始化最大/最小值跟踪数组** — 将 max 清零, min 置为 0xFFFF |

### 22.4 `LTC6811_max_min()`

| 属性 | 内容 |
|------|------|
| **声明** | `void LTC6811_max_min(uint8_t total_ic, cell_asic ic_cells[], cell_asic ic_min[], cell_asic ic_max[], cell_asic ic_delta[])` |
| **功能** | **计算最大/最小值及差值** — 逐电芯跟踪并计算 max、min 和 delta = max - min |

---

## 23. 硬件抽象层函数 (HAL)（内部）

> **调用层级**: 仅库内部使用，应用代码**不直接调用**  
> **文件**: [bms_hardware.c](F103_0804_simulink/LTC681x/bms_hardware.c)  
> **说明**: STM32F103 平台的 SPI 通信硬件抽象层，将 ADI 原库的 Arduino 接口替换为 STM32 HAL。

### 23.1 `bms_hardware_init()`

| 属性 | 内容 |
|------|------|
| **声明** | `void bms_hardware_init(void)` |
| **功能** | **初始化 BMS 硬件** — 配置 PA4 为 CS 输出，初始化 SPI1 |
| **硬件** | CS: GPIOA PA4 (推挽输出, 高速); SPI1: PA5=SCK, PA6=MISO, PA7=MOSI |

### 23.2 `cs_low()`

| 属性 | 内容 |
|------|------|
| **声明** | `void cs_low(uint8_t pin)` |
| **功能** | **拉低片选信号** (CS -> LOW)，选中 SPI 从设备 |
| **注意** | `pin` 参数在当前实现中未使用，CS 固定为 PA4 |

### 23.3 `cs_high()`

| 属性 | 内容 |
|------|------|
| **声明** | `void cs_high(uint8_t pin)` |
| **功能** | **拉高片选信号** (CS -> HIGH)，释放 SPI 从设备 |
| **注意** | `pin` 参数在当前实现中未使用 |

### 23.4 `delay_u()`

| 属性 | 内容 |
|------|------|
| **声明** | `void delay_u(uint16_t micro)` |
| **功能** | **微秒级延迟** — 基于 SystemCoreClock 的忙等待循环 |
| **参数** | `micro` — 微秒数 |

### 23.5 `delay_m()`

| 属性 | 内容 |
|------|------|
| **声明** | `void delay_m(uint16_t milli)` |
| **功能** | **毫秒级延迟** — 调用 `HAL_Delay()` |
| **参数** | `milli` — 毫秒数 |

### 23.6 `set_spi_freq()`

| 属性 | 内容 |
|------|------|
| **声明** | `void set_spi_freq(void)` |
| **功能** | **设置 SPI 频率** — 当前为空实现，SPI 波特率在 `MX_SPI1_Init()` 中配置 |

### 23.7 `spi_write_array()`

| 属性 | 内容 |
|------|------|
| **声明** | `void spi_write_array(uint8_t len, uint8_t data[])` |
| **功能** | **SPI 写入字节数组** — 逐字节发送 |
| **参数** | `len` — 长度; `data[]` — 数据 |

### 23.8 `spi_write_read()`

| 属性 | 内容 |
|------|------|
| **声明** | `void spi_write_read(uint8_t tx_Data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len)` |
| **功能** | **SPI 先写后读** — 先发送 tx_len 字节命令，然后接收 rx_len 字节数据 |
| **参数** | `tx_Data[]` — 发送数据; `tx_len` — 发送长度; `rx_data` — 接收缓冲区; `rx_len` — 接收长度 |

### 23.9 `spi_read_byte()`

| 属性 | 内容 |
|------|------|
| **声明** | `uint8_t spi_read_byte(uint8_t tx_dat)` |
| **功能** | **SPI 读取单字节** — 发送 0xFF 并返回接收字节 |
| **参数** | `tx_dat` — 未使用 (总是发送 0xFF) |
| **返回值** | 接收到的字节 |

---

## 24. LTC6813 完整 API 速查表

> **以下是你实际调用的全部 LTC6813 API 函数，按功能分类。**

### 配置寄存器 (4 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_wrcfg(total_ic, ic)` | 写入 CFGRA | → `LTC681x_wrcfg` |
| `LTC6813_wrcfgb(total_ic, ic)` | 写入 CFGRB | → `LTC681x_wrcfgb` |
| `LTC6813_rdcfg(total_ic, ic)` | 读取 CFGRA | → `LTC681x_rdcfg` |
| `LTC6813_rdcfgb(total_ic, ic)` | 读取 CFGRB | → `LTC681x_rdcfgb` |

### ADC 转换启动 (5 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_adcv(MD, DCP, CH)` | 启动电芯电压转换 | → `LTC681x_adcv` |
| `LTC6813_adax(MD, CHG)` | 启动 GPIO/Vref2 转换 | → `LTC681x_adax` |
| `LTC6813_adstat(MD, CHST)` | 启动状态转换 | → `LTC681x_adstat` |
| `LTC6813_adcvax(MD, DCP)` | CV + GPIO1&2 联合转换 | → `LTC681x_adcvax` |
| `LTC6813_adcvsc(MD, DCP)` | CV + SOC 联合转换 | → `LTC681x_adcvsc` |

### ADC 轮询 (2 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_pladc()` | 查询 ADC 是否完成 | → `LTC681x_pladc` |
| `LTC6813_pollAdc()` | 阻塞等待 ADC 完成 | → `LTC681x_pollAdc` |

### 数据读取与解析 (3 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_rdcv(reg, total_ic, ic)` | 读取解析电芯电压 | → `LTC681x_rdcv` |
| `LTC6813_rdaux(reg, nIC, ic)` | 读取解析 AUX/GPIO | → `LTC681x_rdaux` |
| `LTC6813_rdstat(reg, total_ic, ic)` | 读取解析状态 | → `LTC681x_rdstat` |

### 寄存器清除 (4 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_clrcell()` | 清除电芯电压寄存器 | → `LTC681x_clrcell` |
| `LTC6813_clraux()` | 清除 AUX 寄存器 | → `LTC681x_clraux` |
| `LTC6813_clrstat()` | 清除状态寄存器 | → `LTC681x_clrstat` |
| `LTC6813_clrsctrl()` | 清除 SCTRL 寄存器 | → `LTC681x_clrsctrl` |

### 自检与诊断 (4+3 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_diagn()` | Mux 解码器诊断 | → `LTC681x_diagn` |
| `LTC6813_cvst(MD, ST)` | 电芯电压自检 | → `LTC681x_cvst` |
| `LTC6813_axst(MD, ST)` | AUX 自检 | → `LTC681x_axst` |
| `LTC6813_statst(MD, ST)` | 状态自检 | → `LTC681x_statst` |
| `LTC6813_run_cell_adc_st(adc_reg, total_ic, ic, md, adcopt)` | 运行 ADC 自检 | → `LTC681x_run_cell_adc_st` |
| `LTC6813_run_adc_overlap(total_ic, ic)` | 运行重叠测试 | **独立实现**（18 节多检测一对） |
| `LTC6813_run_adc_redundancy_st(adc_mode, adc_reg, total_ic, ic)` | 运行冗余自检 | → `LTC681x_run_adc_redundancy_st` |

### 冗余/重叠测试启动 (3 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_adol(MD, DCP)` | 重叠转换 | → `LTC681x_adol` |
| `LTC6813_adaxd(MD, CHG)` | GPIO 冗余测试 | → `LTC681x_adaxd` |
| `LTC6813_adstatd(MD, CHST)` | 状态冗余测试 | → `LTC681x_adstatd` |

### 开路检测 (5 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_adow(MD, PUP, CH, DCP)` | 启动开路检测转换 | → `LTC681x_adow` |
| `LTC6813_axow(MD, PUP)` | 启动 GPIO 开路检测 | → `LTC681x_axow` |
| `LTC6813_run_openwire_single(total_ic, ic)` | 单节开路检测 | → `LTC681x_run_openwire_single` |
| `LTC6813_run_openwire_multi(total_ic, ic)` | 多节开路检测 | → `LTC681x_run_openwire_multi` |
| `LTC6813_run_gpio_openwire(total_ic, ic)` | GPIO 开路检测 | → `LTC681x_run_gpio_openwire` |

### 放电控制 (2 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_set_discharge(Cell, total_ic, ic)` | 设置单个电芯放电 | **独立实现**（18 路覆盖 CFGRA+CFGRB） |
| `LTC6813_clear_discharge(total_ic, ic)` | 清除所有放电位 | → `LTC681x_clear_discharge` |

### PWM 寄存器 (2 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_wrpwm(total_ic, pwmReg, ic)` | 写入 PWM 寄存器 | → `LTC681x_wrpwm` |
| `LTC6813_rdpwm(total_ic, pwmReg, ic)` | 读取 PWM 寄存器 | → `LTC681x_rdpwm` |

### SCTRL 寄存器 (4 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_wrsctrl(nIC, sctrl_reg, ic)` | 写入 SCTRL | → `LTC681x_wrsctrl` |
| `LTC6813_rdsctrl(nIC, sctrl_reg, ic)` | 读取 SCTRL | → `LTC681x_rdsctrl` |
| `LTC6813_stsctrl()` | 启动 SCTRL 脉冲通信 | → `LTC681x_stsctrl` |
| `LTC6813_clrsctrl()` | 清除 SCTRL | → `LTC681x_clrsctrl` |

### COMM 寄存器 (3 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_wrcomm(total_ic, ic)` | 写入 COMM | → `LTC681x_wrcomm` |
| `LTC6813_rdcomm(total_ic, ic)` | 读取 COMM | → `LTC681x_rdcomm` |
| `LTC6813_stcomm(len)` | 启动 COMM 数据输出 | → `LTC681x_stcomm` |

### PEC 错误管理 (2 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_check_pec(total_ic, reg, ic)` | 累加 PEC 错误计数 | → `LTC681x_check_pec` |
| `LTC6813_reset_crc_count(total_ic, ic)` | 重置 PEC 计数器 | → `LTC681x_reset_crc_count` |

### CFGRA 配置辅助 (9 个)

| API 函数 | 功能 | 委托 |
|----------|------|------|
| `LTC6813_init_cfg(total_ic, ic)` | 初始化 CFGRA 数据结构 | → `LTC681x_init_cfg` |
| `LTC6813_set_cfgr(nIC, ic, refon, adcopt, gpio, dcc, dcto, uv, ov)` | 综合设置 CFGRA | 调用子函数 |
| `LTC6813_set_cfgr_refon(nIC, ic, refon)` | 设置 REFON 位 | → `LTC681x_set_cfgr_refon` |
| `LTC6813_set_cfgr_adcopt(nIC, ic, adcopt)` | 设置 ADCOPT 位 | → `LTC681x_set_cfgr_adcopt` |
| `LTC6813_set_cfgr_gpio(nIC, ic, gpio)` | 设置 GPIO 方向 | → `LTC681x_set_cfgr_gpio` |
| `LTC6813_set_cfgr_dis(nIC, ic, dcc)` | 设置 DCC 放电位 | → `LTC681x_set_cfgr_dis` |
| `LTC6813_set_cfgr_uv(nIC, ic, uv)` | 设置欠压阈值 | → `LTC681x_set_cfgr_uv` |
| `LTC6813_set_cfgr_dcto(nIC, ic, dcto)` | 设置放电超时 | → `LTC681x_set_cfgr_dcto` |
| `LTC6813_set_cfgr_ov(nIC, ic, ov)` | 设置过压阈值 | → `LTC681x_set_cfgr_ov` |

### CFGRB 配置辅助 (7 个，LTC6813 独有)

| API 函数 | 功能 |
|----------|------|
| `LTC6813_init_cfgb(total_ic, ic)` | 初始化 CFGRB 数据结构 |
| `LTC6813_set_cfgrb(nIC, ic, fdrf, dtmen, ps, gpiobits, dccbits)` | 综合设置 CFGRB |
| `LTC6813_set_cfgrb_fdrf(nIC, ic, fdrf)` | 设置快速放电参考 |
| `LTC6813_set_cfgrb_dtmen(nIC, ic, dtmen)` | 设置放电定时器使能 |
| `LTC6813_set_cfgrb_ps(nIC, ic, ps)` | 设置 S 引脚路径选择 |
| `LTC6813_set_cfgrb_gpio_b(nIC, ic, gpiobits)` | 设置 GPIO6-9 方向 |
| `LTC6813_set_cfgrb_dcc_b(nIC, ic, dccbits)` | 设置 DCC 扩展位 |

### LTC6813 专有命令 (5 个)

| API 函数 | 功能 |
|----------|------|
| `LTC6813_init_reg_limits(total_ic, ic)` | 初始化 18 电芯 / 9 AUX / 6 CV 寄存器限制 |
| `LTC6813_mute()` | 静音放电晶体管 |
| `LTC6813_unmute()` | 解除静音 |
| `LTC6813_wrpsb(total_ic, ic)` | 写入 PWM/Sctrl 寄存器 B 组合 |
| `LTC6813_rdpsb(total_ic, ic)` | 读取 PWM/Sctrl 寄存器 B 组合 |

### 统计

| 层级 | 数量 | 说明 |
|------|------|------|
| **LTC6813 API（你调用的）** | **~50 个** | `LTC6813_stm32.c` 中定义 |
| LTC681x 核心实现（库内部） | 65 个 | `LTC681x.c` 中定义 |
| SPI 协议层（库内部） | 5 个 | `cmd_68` / `write_68` / `read_68` / `wakeup_*` |
| 硬件抽象层（库内部） | 9 个 | `bms_hardware.c` 中定义 |

---

## 25. ADC 模式与通道选择宏

### ADC 转换模式 (`MD`)

| 宏 | 值 | 正常模式 (ADCOPT=0) | 快速模式 (ADCOPT=1) | 适用场景 |
|----|-----|---------------------|---------------------|----------|
| `MD_422HZ_1KHZ` | 0 | 422 Hz | 1 kHz | 标准精度测量 |
| `MD_27KHZ_14KHZ` | 1 | 27 kHz | 14 kHz | 高速采集 |
| `MD_7KHZ_3KHZ` | 2 | 7 kHz | 3 kHz | 中速采集（常用） |
| `MD_26HZ_2KHZ` | 3 | 26 Hz | 2 kHz | 高精度/低噪声 |

### 电芯通道选择 (`CH`)

| 宏 | 值 | 说明 |
|----|-----|------|
| `CELL_CH_ALL` | 0 | 转换所有电芯通道 |
| `CELL_CH_1and7` | 1 | 转换通道 1 和 7 |
| `CELL_CH_2and8` | 2 | 转换通道 2 和 8 |
| `CELL_CH_3and9` | 3 | 转换通道 3 和 9 |
| `CELL_CH_4and10` | 4 | 转换通道 4 和 10 |
| `CELL_CH_5and11` | 5 | 转换通道 5 和 11 |
| `CELL_CH_6and12` | 6 | 转换通道 6 和 12 |

### AUX/GPIO 通道选择 (`CHG`)

| 宏 | 值 | 说明 |
|----|-----|------|
| `AUX_CH_ALL` | 0 | 转换所有 AUX 通道 |
| `AUX_CH_GPIO1` | 1 | 仅转换 GPIO1 |
| `AUX_CH_GPIO2` | 2 | 仅转换 GPIO2 |
| `AUX_CH_GPIO3` | 3 | 仅转换 GPIO3 |
| `AUX_CH_GPIO4` | 4 | 仅转换 GPIO4 |
| `AUX_CH_GPIO5` | 5 | 仅转换 GPIO5 |
| `AUX_CH_VREF2` | 6 | 仅转换第二参考电压 |

### 状态通道选择 (`CHST`)

| 宏 | 值 | 说明 |
|----|-----|------|
| `STAT_CH_ALL` | 0 | 转换所有状态通道 |
| `STAT_CH_SOC` | 1 | 转换电芯电压之和 (Sum of Cells) |
| `STAT_CH_ITEMP` | 2 | 转换内部温度 |
| `STAT_CH_VREGA` | 3 | 转换模拟电源电压 VREGA |
| `STAT_CH_VREGD` | 4 | 转换数字电源电压 VREGD |

### 寄存器选择 (`reg`)

| 宏 | 值 | 说明 |
|----|-----|------|
| `REG_ALL` | 0 | 读取/操作所有寄存器 |
| `REG_1` | 1 | 读取/操作第 1 个寄存器 |
| `REG_2` | 2 | 读取/操作第 2 个寄存器 |
| `REG_3` | 3 | 读取/操作第 3 个寄存器 |
| `REG_4` | 4 | 读取/操作第 4 个寄存器 |
| `REG_5` | 5 | 读取/操作第 5 个寄存器 |
| `REG_6` | 6 | 读取/操作第 6 个寄存器 |

---

---

> **版权声明**: LTC681x 库版权归 Analog Devices, Inc. (ADI) 所有，遵循 BSD 风格许可证。  
> **文档生成**: 基于 F103_0804_simulink 工程 LTC681x 目录下全部 8 个库源文件的手工分析生成。  
> **调用约定**: 应用代码调用 `LTC6813_*` 系列函数（~50 个），底层 `LTC681x_*` 为核心实现（65 个），`bms_hardware` 为 HAL 层（9 个），均不直接调用。
