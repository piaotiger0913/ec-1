/*
 * File: main.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 01-Jul-2022 19:06:11
 */

/*************************************************************************/
/* This automatically generated example C main file shows how to call    */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

/* Include Files */
#include "../cal_corr.h"
#include "../cal_corr_emxAPI.h"
#include "../cal_corr_terminate.h"
#include "../cal_corr_types.h"
#include "../rt_nonfinite.h"
#include "main.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#define NN 10000

/* Function Declarations */
static emxArray_real_T *argInit_1xUnbounded_real_T(short *buf);

static double argInit_real_T(void);

static void main_cal_corr(short *ref, short *echo);

/* Function Definitions */
/*
 * Arguments    : void
 * Return Type  : emxArray_real_T *
 */
static emxArray_real_T *argInit_1xUnbounded_real_T(short *buf)
{
  emxArray_real_T *result;
  int idx0;
  int idx1;
  short Bvalue;
  /* Set the size of the array.
Change this size to the value that the application requires. */

  result = emxCreate_real_T(1, NN);
  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < 1; idx0++) {
    for (idx1 = 0; idx1 < result->size[1U]; idx1++) {
      /* Set the value of the array element.
Change this value to the value that the application requires. */
      Bvalue = (short)buf[idx1];
      result->data[idx1] = (double)Bvalue;
      // printf("result %d\n",Bvalue);
    }
  }
  return result;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void main_cal_corr(short *ref, short *echo)
{
  emxArray_real_T *play;
  emxArray_real_T *rec;
  double out;

  /* Initialize function 'cal_corr' input arguments. */
  /* Initialize function input argument 'rec'. */
  rec = argInit_1xUnbounded_real_T(echo);
  /* Initialize function input argument 'play'. */
  play = argInit_1xUnbounded_real_T(ref);
  /* Call the entry-point 'cal_corr'. */
  out = cal_corr(rec, play);
  printf("out= %f\n", out);
  // emxDestroyArray_real_T(play);
  // emxDestroyArray_real_T(rec);
}

/*
 * Arguments    : int argc
 *                char **argv
 * Return Type  : int
 */

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  FILE *echo_fd, *ref_fd, *e_fd;
  short echo_buf[NN], ref_buf[NN], e_buf[NN];
  int numRead;
  if (argc != 3) {
    fprintf(stderr, "testecho mic_signal.sw speaker_signal.sw output.sw\n");
    exit(1);
  }
  echo_fd = fopen(argv[2], "rb");
  ref_fd = fopen(argv[1], "rb");
  e_fd = fopen(argv[3], "wb");

  fread(ref_buf, sizeof(short), NN, ref_fd);
  fread(echo_buf, sizeof(short), NN, echo_fd);
  // for (int i = 0; i < NN; i++) {
  //   printf("result %d\n",(short)echo_buf[i]);
  // }
  // printf("result %d\n",numRead);
  fclose(echo_fd);
  fclose(ref_fd);
  /* The initialize function is being called automatically from your entry-point
   * function. So, a call to initialize is not included here. */
  /* Invoke the entry-point functions.
You can call entry-point functions multiple times. */
  main_cal_corr(ref_buf, echo_buf);
  /* Terminate the application.
You do not need to do this more than one time. */
  cal_corr_terminate();

  return 0;
}

/*
 * File trailer for main.c
 *
 * [EOF]
 */
