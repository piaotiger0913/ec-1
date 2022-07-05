
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define NN 128
#define TAIL 1024
#define DENOISE_DB (-90)


void process_delay(const int16_t *ref, const int16_t *ech)
{


    // return 0;
}
int main(int argc, char **argv)
{
    FILE *echo_fd, *ref_fd, *e_fd,*rm_fd,*prm_fd;
    short echo_buf[NN], ref_buf[NN], e_buf[NN];
    SpeexEchoState *st;
    SpeexPreprocessState *den;
    SpeexPreprocessState *den1;
    int sampleRate = 8000;
    int supDB = -50;
    int i;
    float f;
    char *p;
    
    if (argc != 5)
    {
        fprintf(stderr, "testecho mic_signal.sw speaker_signal.sw output.sw\n");
        exit(1);
    }
    long conv = strtol(argv[4], &p, 10);
    printf("%ld\n", conv);
    int RM = conv;
    
    short rm_buf[RM];
    echo_fd = fopen(argv[2], "rb");
    ref_fd = fopen(argv[1], "rb");
    e_fd = fopen(argv[3], "wb");
    rm_fd = fopen("/home/user/Music/remaing.raw", "wb");
    prm_fd = fopen("/home/user/Music/Prm.raw", "wb");

    den = speex_preprocess_state_init(NN, sampleRate);
    i = 1;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DENOISE, &i);
    printf("get is noise %d\n", i);
    int noiseSuppress = DENOISE_DB;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress); // Set the dB of noise
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_GET_NOISE_SUPPRESS, &noiseSuppress);
    printf("get noise DB %d\n", noiseSuppress);
    i = 0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC, &i);
    i = 16000;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
    i = 0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB, &i);
    f = .0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
    f = .0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);

    printf("Initialize the echo cancellation object\n");
    st = speex_echo_state_init(NN, TAIL);
    den1 = speex_preprocess_state_init(NN, sampleRate);
    speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
    speex_preprocess_ctl(den1, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

    printf("process start\n");

    int frmNum = fread(rm_buf, sizeof(short), (size_t)RM, echo_fd);
    printf("remaining frame = %d\n", frmNum);

    while (!feof(ref_fd) && !feof(echo_fd))
    {
        
        fread(ref_buf, sizeof(short), NN, ref_fd);
        // remove noise
        
        speex_preprocess_run(den, ref_buf);
        // echo elimination
        fread(echo_buf, sizeof(short), NN, echo_fd);
        // process_delay(ref_buf, echo_buf);
        speex_echo_cancellation(st, ref_buf, echo_buf, e_buf);
        speex_preprocess_run(den1, e_buf);
        fwrite(e_buf, sizeof(short), NN, e_fd);
        fwrite(echo_buf, sizeof(short), NN, rm_fd);
        fwrite(ref_buf, sizeof(short), NN, prm_fd);
    }
    printf("process end\n");
    speex_echo_state_destroy(st);
    speex_preprocess_state_destroy(den);
    speex_preprocess_state_destroy(den1);
    fclose(e_fd);
    fclose(echo_fd);
    fclose(ref_fd);
    fclose(rm_fd);
    fclose(prm_fd);
    return 0;
}
