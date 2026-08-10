/**
 * @file bms_app.h
 * @brief BMS application API for Simulink C Caller
 *
 * Types are fixed-width for easy C Caller port mapping:
 *   uint8_t  -> uint8
 *   uint16_t -> uint16
 *
 * Array outputs: set C Caller output dimension to the BMS_*_LEN macros below.
 * Function return (uint8_t) — NOT the measurement data:
 *   Sample*:  0 = OK, 1 = PEC/read fault
 *   Detect*:  0 = no open wire, 1 = at least one open
 * Actual voltages / flags are in the output arrays.
 *
 * Keil: USE_HAL_DRIVER defined -> real LTC6813.
 * Simulink Simulation Target: stubs (fill zeros / return 0).
 */
#ifndef BMS_APP_H
#define BMS_APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BMS_TOTAL_IC
#define BMS_TOTAL_IC 6U
#endif

#define BMS_CELLS_PER_IC       18U
#define BMS_GPIO_PER_IC        9U
#define BMS_CELL_OW_PER_IC     19U  /* C0 .. C18 */
#define BMS_GPIO_OW_PER_IC     9U   /* GPIO1 .. GPIO9 */

/** Flattened buffer lengths for C Caller vector size */
#define BMS_CELL_CODE_LEN      (BMS_TOTAL_IC * BMS_CELLS_PER_IC)
#define BMS_GPIO_CODE_LEN      (BMS_TOTAL_IC * BMS_GPIO_PER_IC)
#define BMS_CELL_OW_LEN        (BMS_TOTAL_IC * BMS_CELL_OW_PER_IC)
#define BMS_GPIO_OW_LEN        (BMS_TOTAL_IC * BMS_GPIO_OW_PER_IC)

void Bms_LED_Control(uint8_t on);
void Bms_StartCellAdc(void);
void Bms_StartCellAdcEx(uint8_t MD, uint8_t DCP, uint8_t CH);

/**
 * @brief Cell voltage acquire: wakeup -> ADCV(7kHz) -> poll -> RDCV
 * @param codes  [out] length BMS_CELL_CODE_LEN
 *               layout: IC0 cell0..17, IC1 cell0..17, ...
 *               unit: 100 uV (same as LTC681x c_codes)
 * @return 0 OK; 1 PEC/read fault. Voltages are in codes[], not in return.
 *
 * C Caller: Output uint16 vector size = BMS_CELL_CODE_LEN (108 if TOTAL_IC=6)
 *           Return uint8 status
 */
uint8_t Bms_SampleCellVoltage(uint16_t codes[BMS_CELL_CODE_LEN]);

/**
 * @brief Temperature / GPIO acquire: wakeup -> ADAX(7kHz,ALL) -> poll -> RDAUX
 * @param codes  [out] length BMS_GPIO_CODE_LEN
 *               layout: IC0 a_codes[0..8], IC1 ...
 *               (GPIO1..5, VREF2, GPIO6.. typically; raw 100 uV)
 * @return 0 OK; 1 PEC/read fault. Codes are in codes[], not in return.
 *
 * C Caller: Output uint16 vector size = BMS_GPIO_CODE_LEN (54 if TOTAL_IC=6)
 */
uint8_t Bms_SampleTemperature(uint16_t codes[BMS_GPIO_CODE_LEN]);

/**
 * @brief Cell open-wire detect (multi algorithm)
 * @param flags  [out] length BMS_CELL_OW_LEN
 *               layout: IC0 C0..C18, IC1 ...; 1=open, 0=ok
 * @return 0 no open wire; 1 at least one open. Details in flags[].
 *
 * C Caller: Output uint8 vector size = BMS_CELL_OW_LEN (114 if TOTAL_IC=6)
 */
uint8_t Bms_DetectCellOpenWire(uint8_t flags[BMS_CELL_OW_LEN]);

/**
 * @brief Temperature / GPIO open-wire detect
 * @param flags  [out] length BMS_GPIO_OW_LEN
 *               layout: IC0 GPIO1..9, IC1 ...; 1=open, 0=ok
 * @return 0 no open wire; 1 at least one open. Details in flags[].
 *
 * C Caller: Output uint8 vector size = BMS_GPIO_OW_LEN (54 if TOTAL_IC=6)
 */
uint8_t Bms_DetectTempOpenWire(uint8_t flags[BMS_GPIO_OW_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* BMS_APP_H */
