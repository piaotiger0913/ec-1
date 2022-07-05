/*
 * File: _coder_cal_corr_mex.h
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 01-Jul-2022 19:06:11
 */

#ifndef _CODER_CAL_CORR_MEX_H
#define _CODER_CAL_CORR_MEX_H

/* Include Files */
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function Declarations */
MEXFUNCTION_LINKAGE void mexFunction(int32_T nlhs, mxArray *plhs[],
                                     int32_T nrhs, const mxArray *prhs[]);

emlrtCTX mexFunctionCreateRootTLS(void);

void unsafe_cal_corr_mexFunction(int32_T nlhs, mxArray *plhs[1], int32_T nrhs,
                                 const mxArray *prhs[2]);

#ifdef __cplusplus
}
#endif

#endif
/*
 * File trailer for _coder_cal_corr_mex.h
 *
 * [EOF]
 */
