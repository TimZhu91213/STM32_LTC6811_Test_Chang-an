/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: F103_0804_simulink_model.h
 *
 * Code generated for Simulink model 'F103_0804_simulink_model'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Sun Aug  9 23:06:44 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->STM32Processor
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef F103_0804_simulink_model_h_
#define F103_0804_simulink_model_h_
#ifndef F103_0804_simulink_model_COMMON_INCLUDES_
#define F103_0804_simulink_model_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                           /* F103_0804_simulink_model_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#define F103_0804_simulink_model_M     (rtM)

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* user code (top of header file) */
#include "bms_app.h"

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  uint16_T CCaller2_o2[108];           /* '<S1>/C Caller2' */
  uint16_T CCaller1_o2[54];            /* '<S1>/C Caller1' */
  uint8_T CCaller3_o2[114];            /* '<S1>/C Caller3' */
  uint8_T CCaller4_o2[54];             /* '<S1>/C Caller4' */
  uint8_T is_active_c3_F103_0804_simulink;/* '<S2>/Chart' */
  uint8_T is_c3_F103_0804_simulink_model;/* '<S2>/Chart' */
  uint8_T temporalCounter_i1;          /* '<S2>/Chart' */
} DW;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* Model entry point functions */
extern void F103_0804_simulink_model_initialize(void);

/* Exported entry point function */
extern void Sample_function_call(void);/* Explicit Task: Sample_function_call */

/* Exported entry point function */
extern void LED_function_call(void);   /* Explicit Task: LED_function_call */

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S2>/Data Type Conversion' : Eliminate redundant data type conversion
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'F103_0804_simulink_model'
 * '<S1>'   : 'F103_0804_simulink_model/Open_Wire_Check_and_Sample'
 * '<S2>'   : 'F103_0804_simulink_model/Subsystem'
 * '<S3>'   : 'F103_0804_simulink_model/Subsystem/Chart'
 */
#endif                                 /* F103_0804_simulink_model_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
