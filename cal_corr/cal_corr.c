/*
 * File: cal_corr.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 01-Jul-2022 19:06:11
 */

/* Include Files */
#include "cal_corr.h"
#include "FFTImplementationCallback.h"
#include "cal_corr_emxutil.h"
#include "cal_corr_types.h"
#include "fft.h"
#include "rt_nonfinite.h"
#include "rt_nonfinite.h"
#include <math.h>

/* Function Definitions */
/*
 * Arguments    : const emxArray_real_T *rec
 *                const emxArray_real_T *play
 * Return Type  : double
 */
double cal_corr(const emxArray_real_T *rec, const emxArray_real_T *play)
{
  emxArray_creal_T b_x;
  emxArray_creal_T *b_fv;
  emxArray_creal_T *fv;
  emxArray_creal_T *wwc;
  emxArray_creal_T *x;
  emxArray_creal_T *y;
  emxArray_creal_T *yCol;
  emxArray_real_T *costab;
  emxArray_real_T *costab1q;
  emxArray_real_T *sintab;
  emxArray_real_T *sintabinv;
  double ai;
  double nt_im;
  double nt_re;
  int N2blue;
  int b_y;
  int c_x;
  int i;
  int k;
  int nInt2;
  int nd2;
  int nfft;
  int rt;
  bool exitg1;
  bool useRadix2;
  emxInit_creal_T(&y, 2);
  emxInit_creal_T(&x, 2);
  fft(rec, x);
  fft(play, y);
  i = y->size[0] * y->size[1];
  y->size[0] = 1;
  emxEnsureCapacity_creal_T(y, i);
  nd2 = y->size[1] - 1;
  for (i = 0; i <= nd2; i++) {
    y->data[i].im = -y->data[i].im;
  }
  i = x->size[0] * x->size[1];
  x->size[0] = 1;
  emxEnsureCapacity_creal_T(x, i);
  nd2 = x->size[1] - 1;
  for (i = 0; i <= nd2; i++) {
    nt_im = x->data[i].re * y->data[i].im + x->data[i].im * y->data[i].re;
    x->data[i].re =
        x->data[i].re * y->data[i].re - x->data[i].im * y->data[i].im;
    x->data[i].im = nt_im;
  }
  nfft = x->size[1];
  emxInit_real_T(&costab1q, 2);
  if (x->size[1] == 0) {
    y->size[0] = 1;
    y->size[1] = 0;
  } else {
    useRadix2 = ((x->size[1] & (x->size[1] - 1)) == 0);
    c_FFTImplementationCallback_get(x->size[1], useRadix2, &N2blue, &nd2);
    nt_im = 6.2831853071795862 / (double)nd2;
    nInt2 = nd2 / 2 / 2;
    i = costab1q->size[0] * costab1q->size[1];
    costab1q->size[0] = 1;
    costab1q->size[1] = nInt2 + 1;
    emxEnsureCapacity_real_T(costab1q, i);
    costab1q->data[0] = 1.0;
    nd2 = nInt2 / 2 - 1;
    for (k = 0; k <= nd2; k++) {
      costab1q->data[k + 1] = cos(nt_im * ((double)k + 1.0));
    }
    i = nd2 + 2;
    rt = nInt2 - 1;
    for (k = i; k <= rt; k++) {
      costab1q->data[k] = sin(nt_im * (double)(nInt2 - k));
    }
    costab1q->data[nInt2] = 0.0;
    emxInit_real_T(&costab, 2);
    emxInit_real_T(&sintab, 2);
    emxInit_real_T(&sintabinv, 2);
    if (!useRadix2) {
      nInt2 = costab1q->size[1] - 1;
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
      for (k = 0; k < nInt2; k++) {
        sintabinv->data[k + 1] = costab1q->data[(nInt2 - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        sintabinv->data[k] = costab1q->data[k - nInt2];
      }
      for (k = 0; k < nInt2; k++) {
        costab->data[k + 1] = costab1q->data[k + 1];
        sintab->data[k + 1] = -costab1q->data[(nInt2 - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        costab->data[k] = -costab1q->data[nd2 - k];
        sintab->data[k] = -costab1q->data[k - nInt2];
      }
    } else {
      nInt2 = costab1q->size[1] - 1;
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
      for (k = 0; k < nInt2; k++) {
        costab->data[k + 1] = costab1q->data[k + 1];
        sintab->data[k + 1] = costab1q->data[(nInt2 - k) - 1];
      }
      i = costab1q->size[1];
      for (k = i; k <= nd2; k++) {
        costab->data[k] = -costab1q->data[nd2 - k];
        sintab->data[k] = costab1q->data[k - nInt2];
      }
      sintabinv->size[0] = 1;
      sintabinv->size[1] = 0;
    }
    emxInit_creal_T(&yCol, 1);
    if (useRadix2) {
      nd2 = x->size[1];
      b_x = *x;
      c_x = nd2;
      b_x.size = &c_x;
      b_x.numDimensions = 1;
      c_FFTImplementationCallback_r2b(&b_x, x->size[1], costab, sintab, yCol);
      if (yCol->size[0] > 1) {
        nt_im = 1.0 / (double)yCol->size[0];
        nd2 = yCol->size[0];
        for (i = 0; i < nd2; i++) {
          yCol->data[i].re *= nt_im;
          yCol->data[i].im *= nt_im;
        }
      }
    } else {
      emxInit_creal_T(&wwc, 1);
      nd2 = (x->size[1] + x->size[1]) - 1;
      i = wwc->size[0];
      wwc->size[0] = nd2;
      emxEnsureCapacity_creal_T(wwc, i);
      rt = 0;
      wwc->data[x->size[1] - 1].re = 1.0;
      wwc->data[x->size[1] - 1].im = 0.0;
      nInt2 = x->size[1] << 1;
      i = x->size[1];
      for (k = 0; k <= i - 2; k++) {
        b_y = ((k + 1) << 1) - 1;
        if (nInt2 - rt <= b_y) {
          rt += b_y - nInt2;
        } else {
          rt += b_y;
        }
        nt_im = 3.1415926535897931 * (double)rt / (double)nfft;
        if (nt_im == 0.0) {
          nt_re = 1.0;
          nt_im = 0.0;
        } else {
          nt_re = cos(nt_im);
          nt_im = sin(nt_im);
        }
        wwc->data[(x->size[1] - k) - 2].re = nt_re;
        wwc->data[(x->size[1] - k) - 2].im = -nt_im;
      }
      i = nd2 - 1;
      for (k = i; k >= nfft; k--) {
        wwc->data[k] = wwc->data[(nd2 - k) - 1];
      }
      i = yCol->size[0];
      yCol->size[0] = x->size[1];
      emxEnsureCapacity_creal_T(yCol, i);
      rt = x->size[1];
      for (k = 0; k < rt; k++) {
        nd2 = (nfft + k) - 1;
        nt_re = wwc->data[nd2].re;
        nt_im = wwc->data[nd2].im;
        yCol->data[k].re = nt_re * x->data[k].re + nt_im * x->data[k].im;
        yCol->data[k].im = nt_re * x->data[k].im - nt_im * x->data[k].re;
      }
      i = x->size[1] + 1;
      for (k = i; k <= nfft; k++) {
        yCol->data[k - 1].re = 0.0;
        yCol->data[k - 1].im = 0.0;
      }
      emxInit_creal_T(&fv, 1);
      emxInit_creal_T(&b_fv, 1);
      c_FFTImplementationCallback_r2b(yCol, N2blue, costab, sintab, fv);
      c_FFTImplementationCallback_r2b(wwc, N2blue, costab, sintab, b_fv);
      i = b_fv->size[0];
      b_fv->size[0] = fv->size[0];
      emxEnsureCapacity_creal_T(b_fv, i);
      nd2 = fv->size[0];
      for (i = 0; i < nd2; i++) {
        nt_im = fv->data[i].re * b_fv->data[i].im +
                fv->data[i].im * b_fv->data[i].re;
        b_fv->data[i].re = fv->data[i].re * b_fv->data[i].re -
                           fv->data[i].im * b_fv->data[i].im;
        b_fv->data[i].im = nt_im;
      }
      c_FFTImplementationCallback_r2b(b_fv, N2blue, costab, sintabinv, fv);
      emxFree_creal_T(&b_fv);
      if (fv->size[0] > 1) {
        nt_im = 1.0 / (double)fv->size[0];
        nd2 = fv->size[0];
        for (i = 0; i < nd2; i++) {
          fv->data[i].re *= nt_im;
          fv->data[i].im *= nt_im;
        }
      }
      nt_re = x->size[1];
      i = x->size[1];
      rt = wwc->size[0];
      for (k = i; k <= rt; k++) {
        nt_im = wwc->data[k - 1].re * fv->data[k - 1].re +
                wwc->data[k - 1].im * fv->data[k - 1].im;
        ai = wwc->data[k - 1].re * fv->data[k - 1].im -
             wwc->data[k - 1].im * fv->data[k - 1].re;
        if (ai == 0.0) {
          nd2 = k - i;
          yCol->data[nd2].re = nt_im / nt_re;
          yCol->data[nd2].im = 0.0;
        } else if (nt_im == 0.0) {
          nd2 = k - i;
          yCol->data[nd2].re = 0.0;
          yCol->data[nd2].im = ai / nt_re;
        } else {
          nd2 = k - i;
          yCol->data[nd2].re = nt_im / nt_re;
          yCol->data[nd2].im = ai / nt_re;
        }
      }
      emxFree_creal_T(&fv);
      emxFree_creal_T(&wwc);
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
  emxFree_creal_T(&x);
  i = costab1q->size[0] * costab1q->size[1];
  costab1q->size[0] = 1;
  costab1q->size[1] = y->size[1];
  emxEnsureCapacity_real_T(costab1q, i);
  nd2 = y->size[1];
  for (i = 0; i < nd2; i++) {
    costab1q->data[i] = y->data[i].re;
  }
  emxFree_creal_T(&y);
  /*  acorr = real(ifft(fft(play).*conj(fft(play)))); */
  nd2 = costab1q->size[1];
  if (costab1q->size[1] <= 2) {
    if (costab1q->size[1] == 1) {
      rt = 1;
    } else if ((costab1q->data[0] < costab1q->data[costab1q->size[1] - 1]) ||
               (rtIsNaN(costab1q->data[0]) &&
                (!rtIsNaN(costab1q->data[costab1q->size[1] - 1])))) {
      rt = costab1q->size[1];
    } else {
      rt = 1;
    }
  } else {
    if (!rtIsNaN(costab1q->data[0])) {
      rt = 1;
    } else {
      rt = 0;
      k = 2;
      exitg1 = false;
      while ((!exitg1) && (k <= nd2)) {
        if (!rtIsNaN(costab1q->data[k - 1])) {
          rt = k;
          exitg1 = true;
        } else {
          k++;
        }
      }
    }
    if (rt == 0) {
      rt = 1;
    } else {
      nt_im = costab1q->data[rt - 1];
      i = rt + 1;
      for (k = i; k <= nd2; k++) {
        ai = costab1q->data[k - 1];
        if (nt_im < ai) {
          nt_im = ai;
          rt = k;
        }
      }
    }
  }
  emxFree_real_T(&costab1q);
  return rt;
}

/*
 * File trailer for cal_corr.c
 *
 * [EOF]
 */
