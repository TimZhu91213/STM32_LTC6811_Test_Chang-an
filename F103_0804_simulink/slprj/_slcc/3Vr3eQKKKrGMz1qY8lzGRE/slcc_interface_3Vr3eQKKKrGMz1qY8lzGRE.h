#include "customcode_3Vr3eQKKKrGMz1qY8lzGRE.h"
#ifdef __cplusplus
extern "C" {
#endif


/* Type Definitions */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
DLL_EXPORT_CC extern const char_T *get_dll_checksum_3Vr3eQKKKrGMz1qY8lzGRE(void);
DLL_EXPORT_CC extern void Bms_LED_Control_3Vr3eQKKKrGMz1qY8lzGRE(uint8_T on);
DLL_EXPORT_CC extern void Bms_StartCellAdc_3Vr3eQKKKrGMz1qY8lzGRE(void);
DLL_EXPORT_CC extern void Bms_StartCellAdcEx_3Vr3eQKKKrGMz1qY8lzGRE(uint8_T MD, uint8_T DCP, uint8_T CH);
DLL_EXPORT_CC extern uint8_T Bms_SampleCellVoltage_3Vr3eQKKKrGMz1qY8lzGRE(uint16_T codes[108]);
DLL_EXPORT_CC extern uint8_T Bms_SampleTemperature_3Vr3eQKKKrGMz1qY8lzGRE(uint16_T codes[54]);
DLL_EXPORT_CC extern uint8_T Bms_DetectCellOpenWire_3Vr3eQKKKrGMz1qY8lzGRE(uint8_T flags[114]);
DLL_EXPORT_CC extern uint8_T Bms_DetectTempOpenWire_3Vr3eQKKKrGMz1qY8lzGRE(uint8_T flags[54]);

/* Function Definitions */
DLL_EXPORT_CC const uint8_T *get_checksum_source_info(int32_T *size);
#ifdef __cplusplus
}
#endif

