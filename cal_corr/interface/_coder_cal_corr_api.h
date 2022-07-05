/*
 * File: _coder_cal_corr_api.h
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 01-Jul-2022 19:06:11
 */

#ifndef _CODER_CAL_CORR_API_H
#define _CODER_CAL_CORR_API_H

/* Include Files */
#include "emlrt.h"
#include "tmwtypes.h"
#include <string.h>

/* Type Definitions */
#ifndef struct_emxArray_real_T
#define struct_emxArray_real_T
struct emxArray_real_T {
  real_T *data;
  int32_T *size;
  int32_T allocatedSize;
  int32_T numDimensions;
  boolean_T canFreeData;
};
#endif /* struct_emxArray_real_T */
#ifndef typedef_emxArray_real_T
#define typedef_emxArray_real_T
typedef struct emxArray_real_T emxArray_real_T;
#endif /* typedef_emxArray_real_T */

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

#ifdef __cplusplus
extern "C" {
#endif

/* Function Declarations */
real_T cal_corr(emxArray_real_T *rec, emxArray_real_T *play);

void cal_corr_api(const mxArray *const prhs[2], const mxArray **plhs);

void cal_corr_atexit(void);

void cal_corr_initialize(void);

void cal_corr_terminate(void);

void cal_corr_xil_shutdown(void);

void cal_corr_xil_terminate(void);

#ifdef __cplusplus
}
#endif

#endif
/*
 * File trailer for _coder_cal_corr_api.h
 *
 * [EOF]
 */
