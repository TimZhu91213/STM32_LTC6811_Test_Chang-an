/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: F103_0804_simulink_model.c
 *
 * Code generated for Simulink model 'F103_0804_simulink_model'.
 *
 * Model version                  : 1.3
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Mon Aug 10 16:58:34 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->STM32Processor
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "F103_0804_simulink_model.h"
#include "rtwtypes.h"

/* Named constants for Chart: '<S3>/Chart' */
#define IN_Error_OFF                   ((uint8_T)1U)
#define IN_Error_ON                    ((uint8_T)2U)
#define IN_OFF                         ((uint8_T)3U)
#define IN_ON                          ((uint8_T)4U)

/* Block signals and states (default storage) */
DW rtDW;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function for TID1 */
void Sample_function_call(void)        /* Explicit Task: Sample_function_call */
{
  /* RootInportFunctionCallGenerator generated from: '<Root>/Sample_function_call' incorporates:
   *  SubSystem: '<Root>/Function-Call Subsystem'
   */
  /* Switch: '<S1>/Switch' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtDW.Switch = 1.0;

  /* RootInportFunctionCallGenerator generated from: '<Root>/Sample_function_call' incorporates:
   *  SubSystem: '<Root>/Open_Wire_Check_and_Sample'
   */
  /* CCaller: '<S2>/C Caller3' */
  Bms_DetectCellOpenWire(&rtDW.CCaller3_o2[0]);

  /* CCaller: '<S2>/C Caller4' */
  Bms_DetectTempOpenWire(&rtDW.CCaller4_o2[0]);

  /* CCaller: '<S2>/C Caller2' */
  Bms_SampleCellVoltage(&rtDW.CCaller2_o2[0]);

  /* CCaller: '<S2>/C Caller1' */
  Bms_SampleTemperature(&rtDW.CCaller1_o2[0]);

  /* End of Outputs for RootInportFunctionCallGenerator generated from: '<Root>/Sample_function_call' */
}

/* Model step function for TID2 */
void LED_function_call(void)           /* Explicit Task: LED_function_call */
{
  real_T rtb_UnitDelay;
  uint8_T rtb_out;

  /* RootInportFunctionCallGenerator generated from: '<Root>/LED_function_call' incorporates:
   *  SubSystem: '<Root>/Subsystem'
   */
  /* UnitDelay: '<S3>/Unit Delay' */
  rtb_UnitDelay = rtDW.UnitDelay_DSTATE;

  /* Chart: '<S3>/Chart' */
  if (rtDW.temporalCounter_i1 < 127) {
    rtDW.temporalCounter_i1++;
  }

  if (rtDW.is_active_c3_F103_0804_simulink == 0) {
    rtDW.is_active_c3_F103_0804_simulink = 1U;
    rtDW.temporalCounter_i1 = 0U;
    rtDW.is_c3_F103_0804_simulink_model = IN_OFF;
    rtb_out = 0U;
  } else {
    switch (rtDW.is_c3_F103_0804_simulink_model) {
     case IN_Error_OFF:
      rtb_out = 0U;
      if (rtb_UnitDelay == 0.0) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_OFF;
      } else if (rtDW.temporalCounter_i1 >= 10) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_Error_ON;
        rtb_out = 1U;
      }
      break;

     case IN_Error_ON:
      rtb_out = 1U;
      if (rtDW.temporalCounter_i1 >= 10) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_Error_OFF;
        rtb_out = 0U;
      }
      break;

     case IN_OFF:
      rtb_out = 0U;
      if (rtDW.temporalCounter_i1 >= 100) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_ON;
        rtb_out = 1U;
      } else if (rtb_UnitDelay == 1.0) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_Error_OFF;
      }
      break;

     default:
      /* case IN_ON: */
      rtb_out = 1U;
      if (rtDW.temporalCounter_i1 >= 100) {
        rtDW.temporalCounter_i1 = 0U;
        rtDW.is_c3_F103_0804_simulink_model = IN_OFF;
        rtb_out = 0U;
      }
      break;
    }
  }

  /* End of Chart: '<S3>/Chart' */

  /* CCaller: '<S3>/C Caller' */
  Bms_LED_Control(rtb_out);

  /* Update for UnitDelay: '<S3>/Unit Delay' */
  rtDW.UnitDelay_DSTATE = rtDW.Switch;

  /* End of Outputs for RootInportFunctionCallGenerator generated from: '<Root>/LED_function_call' */
}

/* Model initialize function */
void F103_0804_simulink_model_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
