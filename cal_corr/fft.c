/*
 * File: fft.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 01-Jul-2022 19:06:11
 */

/* Include Files */
#include "fft.h"
#include "FFTImplementationCallback.h"
#include "cal_corr_emxutil.h"
#include "cal_corr_types.h"
#include "rt_nonfinite.h"
#include <math.h>

/* Function Definitions */
/*
 * Arguments    : const emxArray_real_T *x
 *                emxArray_creal_T *y
 * Return Type  : void
 */
void fft(const emxArray_real_T *x, emxArray_creal_T *y)
{
  emxArray_creal_T *yCol;
  emxArray_real_T b_x;
  emxArray_real_T *costab;
  emxArray_real_T *costab1q;
  emxArray_real_T *sintab;
  emxArray_real_T *sintabinv;
  double e;
  int N2blue;
  int c_x;
  int d_x;
  int i;
  int k;
  int n;
  int nd2;
  bool useRadix2;
  if (x->size[1] == 0) {
    y->size[0] = 1;
    y->size[1] = 0;
  } else {
    emxInit_real_T(&costab1q, 2);
    useRadix2 = ((x->size[1] & (x->size[1] - 1)) == 0);
    c_FFTImplementationCallback_get(x->size[1], useRadix2, &N2blue, &nd2);
    e = 6.2831853071795862 / (double)nd2;
    n = nd2 / 2 / 2;
    i = costab1q->size[0] * costab1q->size[1];
    costab1q->size[0] = 1;
    costab1q->size[1] = n + 1;
    emxEnsureCapacity_real_T(costab1q, i);
    costab1q->data[0] = 1.0;
    nd2 = n / 2 - 1;
    for (k = 0; k <= nd2; k++) {
      costab1q->data[k + 1] = cos(e * ((double)k + 1.0));
    }
    i = nd2 + 2;
    nd2 = n - 1;
    for (k = i; k <= nd2; k++) {
      costab1q->data[k] = sin(e * (double)(n - k));
    }
    costab1q->data[n] = 0.0;
    emxInit_real_T(&costab, 2);
    emxInit_real_T(&sintab, 2);
    emxInit_real_T(&sintabinv, 2);
    if (!useRadix2) {
      n = costab1q->size[1] - 1;
      nd2 = (costab1q->size[1] - 1) << 1;
      i = costab->size[0] * costab->size[1];
      costab->size[0] = 1;
      costab->size[1] = nd2 + 1;
      emxEnsureCapacity_real_T(costab, i);
      i = sintab->size[0] * sintab->size[1];
      sintab->size[0] = 1;
      sintab->size[1] = nd2 + 1;
      emxEnsureCapacity_real_T(sintab, i);
      costab->data[0] = 1.0;
      sintab->data[0] = 0.0;
      i = sintabinv->size[0] * sintabinv->size[1];
      sintabinv->size[0] = 1;
      sintabinv->size[1] = nd2 + 1;
      emxEnsureCapacity_real_T(sintabinv, i);
      for (k = 0; k < n; k++) {
        sintabinv->data[k + 1] = costab1q->data[(n - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        sintabinv->data[k] = costab1q->data[k - n];
      }
      for (k = 0; k < n; k++) {
        costab->data[k + 1] = costab1q->data[k + 1];
        sintab->data[k + 1] = -costab1q->data[(n - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        costab->data[k] = -costab1q->data[nd2 - k];
        sintab->data[k] = -costab1q->data[k - n];
      }
    } else {
      n = costab1q->size[1] - 1;
      nd2 = (costab1q->size[1] - 1) << 1;
      i = costab->size[0] * costab->size[1];
      costab->size[0] = 1;
      costab->size[1] = nd2 + 1;
      emxEnsureCapacity_real_T(costab, i);
      i = sintab->size[0] * sintab->size[1];
      sintab->size[0] = 1;
      sintab->size[1] = nd2 + 1;
      emxEnsureCapacity_real_T(sintab, i);
      costab->data[0] = 1.0;
      sintab->data[0] = 0.0;
      for (k = 0; k < n; k++) {
        costab->data[k + 1] = costab1q->data[k + 1];
        sintab->data[k + 1] = -costab1q->data[(n - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        costab->data[k] = -costab1q->data[nd2 - k];
        sintab->data[k] = -costab1q->data[k - n];
      }
      sintabinv->size[0] = 1;
      sintabinv->size[1] = 0;
    }
    emxFree_real_T(&costab1q);
    emxInit_creal_T(&yCol, 1);
    if (useRadix2) {
      i = yCol->size[0];
      yCol->size[0] = x->size[1];
      emxEnsureCapacity_creal_T(yCol, i);
      if (x->size[1] != 1) {
        nd2 = x->size[1];
        b_x = *x;
        d_x = nd2;
        b_x.size = &d_x;
        b_x.numDimensions = 1;
        c_FFTImplementationCallback_doH(&b_x, yCol, x->size[1], costab, sintab);
      } else {
        yCol->data[0].re = x->data[0];
        yCol->data[0].im = 0.0;
      }
    } else {
      nd2 = x->size[1];
      b_x = *x;
      c_x = nd2;
      b_x.size = &c_x;
      b_x.numDimensions = 1;
      c_FFTImplementationCallback_dob(&b_x, N2blue, x->size[1], costab, sintab,
                                      sintabinv, yCol);
    }
    emxFree_real_T(&sintabinv);
    emxFree_real_T(&sintab);
    emxFree_real_T(&costab);
    i = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = x->size[1];
    emxEnsureCapacity_creal_T(y, i);
    nd2 = x->size[1];
    for (i = 0; i < nd2; i++) {
      y->data[i] = yCol->data[i];
    }
    emxFree_creal_T(&yCol);
  }
}

/*
 * File trailer for fft.c
 *
 * [EOF]
 */
