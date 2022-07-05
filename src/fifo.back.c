#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"


#define NN 128
#define TAIL 1024

int main(int argc, char **argv)
{
   FILE *echo_fd, *ref_fd, *e_fd;
   short echo_buf[NN], ref_buf[NN], e_buf[NN];
   SpeexEchoState *st;
   SpeexPreprocessState *den;
   int sampleRate = 8000;

   if (argc != 4)
   {
      fprintf(stderr, "testecho mic_signal.sw speaker_signal.sw output.sw\n");
      exit(1);
   }
   echo_fd = fopen(argv[2], "rb");
   ref_fd  = fopen(argv[1],  "rb");
   e_fd    = fopen(argv[3], "wb");
   // Step1: 初始化結構
     st = speex_echo_state_init(NN, TAIL);
   den = speex_preprocess_state_init(NN, sampleRate);

   //Step2: 設置相關參數
   speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
   speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

   while (!feof(ref_fd) && !feof(echo_fd))
   {
      fread(ref_buf, sizeof(short), NN, ref_fd);
      fread(echo_buf, sizeof(short), NN, echo_fd);
      
      //Step3: 調用Api回聲消除，ref_buf是麥克採集到的數據
      // echo_buf：是從speaker處獲取到的數據
      // e_buf: 是回聲消除後的數據
      speex_echo_cancellation(st, ref_buf, echo_buf, e_buf);
      speex_preprocess_run(den, e_buf);
      fwrite(e_buf, sizeof(short), NN, e_fd);
   }

   //Step4: 銷燬結構 釋放資源
   speex_echo_state_destroy(st);
   speex_preprocess_state_destroy(den);
   fclose(e_fd);
   fclose(echo_fd);
   fclose(ref_fd);
   return 0;
}