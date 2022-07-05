// ec - echo canceller

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>

#include <speex/speex_echo.h>
#include <speex/speex_resampler.h>

#include "conf.h"
#include "audio.h"

#include <speex/speex_preprocess.h>

const char *usage =
    "Usage:\n %s [options]\n"
    "Options:\n"
    " -i PCM            playback PCM (default)\n"
    " -o PCM            capture PCM (default)\n"
    " -r rate           sample rate (16000)\n"
    " -c channels       recording channels (2)\n"
    " -b size           buffer size (262144)\n"
    " -d delay          system delay between playback and capture (0)\n"
    " -f filter_length  AEC filter length (2048)\n"
    " -s                save audio to /tmp/playback.raw, /tmp/recording.raw and /tmp/out.raw\n"
    " -D                daemonize\n"
    " -h                display this help text\n"
    "Note:\n"
    " Access audio I/O through named pipes (/tmp/ec.input for playback and /tmp/ec.output for recording)\n"
    "  `cat audio.raw > /tmp/ec.input` to play audio\n"
    "  `cat /tmp/ec.output > out.raw` to get recording audio\n"
    " Only support mono playback\n";

volatile int g_is_quit = 0;

extern int pFlag;

extern int fifo_setup(conf_t *conf);
extern int fifo_write(void *buf, size_t frames);

#define DENOISE_DB (-90)

void int_handler(int signal)
{
    printf("Caught signal %d, quit...\n", signal);

    g_is_quit = 1;
}

int main(int argc, char *argv[])
{
    SpeexEchoState *echo_state;
    int16_t *rec = NULL;
    int16_t *far = NULL;
    int16_t *out = NULL;
    int16_t *re_out = NULL;
    FILE *fp_rec = NULL;
    FILE *fp_far = NULL;
    FILE *fp_out = NULL;

    int opt = 0;
    int delay = 0;
    int save_audio = 0;
    int daemonize = 0;

    conf_t config = {
        .rec_pcm = "default",
        .out_pcm = "default",
        .playback_fifo = "/tmp/ec.input",
        .out_fifo = "/tmp/ec.output",
        .rate = 16000,
        .rec_channels = 1,
        .ref_channels = 1,
        .out_channels = 1,
        .bits_per_sample = 16,
        .buffer_size = 1024 * 16,
        .playback_fifo_size = 1024 * 4,
        .filter_length = 8000,
        .bypass = 0};

    while ((opt = getopt(argc, argv, "b:c:d:Df:hi:o:r:s")) != -1)
    {
        switch (opt)
        {
        case 'b':
            config.buffer_size = atoi(optarg);
            break;
        case 'c':
            config.rec_channels = atoi(optarg);
            config.out_channels = config.rec_channels;
            break;
        case 'd':
            delay = atoi(optarg);
            break;
        case 'D':
            daemonize = 1;
            break;
        case 'f':
            config.filter_length = atoi(optarg);
            break;
        case 'h':
            printf(usage, argv[0]);
            exit(0);
        case 'i':
            config.rec_pcm = optarg;
            break;
        case 'o':
            config.out_pcm = optarg;
            break;
        case 'r':
            config.rate = atoi(optarg);
            break;
        case 's':
            save_audio = 1;
            break;
        case '?':
            printf("\n");
            printf(usage, argv[0]);
            exit(1);
        default:
            break;
        }
    }

    if (daemonize)
    {
        pid_t pid, sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0)
        {
            printf("fork() failed\n");
            exit(1);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0)
        {
            exit(0);
        }

        /* Change the file mode mask */
        umask(0);

        /* Open any logs here */

        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0)
        {
            printf("setsid() failed\n");
            exit(1);
        }

        /* Change the current working directory */
        if ((chdir("/")) < 0)
        {
            printf("chdir() failed\n");
            exit(1);
        }
    }

    int frame_size = config.rate * 30 / 1000; // 10 ms

    if (save_audio)
    {
        fp_far = fopen("/tmp/playback.raw", "wb");
        fp_rec = fopen("/tmp/recording.raw", "wb");
        fp_out = fopen("/tmp/out.raw", "wb");

        if (fp_far == NULL || fp_rec == NULL || fp_out == NULL)
        {
            printf("Fail to open file(s)\n");
            exit(1);
        }
    }

    rec = (int16_t *)calloc(frame_size * config.rec_channels, sizeof(int16_t));
    far = (int16_t *)calloc(frame_size * config.ref_channels, sizeof(int16_t));
    out = (int16_t *)calloc(frame_size * config.out_channels, sizeof(int16_t));
    re_out = (int16_t *)calloc(frame_size * config.out_channels, sizeof(int16_t));

    if (rec == NULL || far == NULL || out == NULL)
    {
        printf("Fail to allocate memory\n");
        exit(1);
    }

    // Configures signal handling.
    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = int_handler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    sigaction(SIGINT, &sig_int_handler, NULL);

    SpeexPreprocessState *den;
    int i;
    float f;
    den = speex_preprocess_state_init(frame_size, config.rate);
    i = 1;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DENOISE, &i);
    printf("get is noise %d\n", i);
    int noiseSuppress = DENOISE_DB;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress); // Set the dB of noise
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_GET_NOISE_SUPPRESS, &noiseSuppress);
    printf("get noise DB %d\n", noiseSuppress);
    i = 0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC, &i);
    i = 8000;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
    i = 0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB, &i);
    f = .0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
    f = .0;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);

    printf("Initialize the echo cancellation object\n");
    echo_state = speex_echo_state_init(frame_size, config.filter_length);
    SpeexPreprocessState *preprocess_state = speex_preprocess_state_init(frame_size, config.rate);
    speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &(config.rate));
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);
    create_mux();
    playback_start(&config);
    capture_start(&config);
    fifo_setup(&config);

    printf("Running... Press Ctrl+C to exit\n");

    int timeout = 200 * 1000 * frame_size / config.rate; // ms

    // resample
    int resampler_err = 0;
    SpeexResamplerState *rec_rs_state =
        speex_resampler_init(1,           // infile_info.channels,
                             config.rate, // infile_info.samplerate,
                             8000,
                             10,
                             &resampler_err);

    // system delay between recording and playback
    printf("skip frames %d\n", capture_skip(delay));
    printf("filter length %d\n", config.filter_length);

    while (!g_is_quit)
    {

        int playNum = playback_read(far, frame_size, timeout);

        int recNum = capture_read(rec, frame_size, timeout);

        if (recNum != playNum)
        {
            printf("error: %d, %d\n", playNum, recNum);
        }

        if (!config.bypass)
        {
            speex_preprocess_run(den, rec);
            // speex_echo_playback(echo_state, far);
            speex_echo_cancellation(echo_state, rec, far, out);
            // printf("%d\n", 1);
            speex_preprocess_run(preprocess_state, out);
            uint32_t in_pro = frame_size;
            uint32_t out_pro = frame_size;
            int err = speex_resampler_process_int(rec_rs_state, 0, out, &in_pro, re_out, &out_pro);
        }
        else
        {
            memcpy(out, rec, frame_size * config.rec_channels * config.bits_per_sample / 8);
        }
        if (fp_far)
        {
            fwrite(rec, 2, frame_size * config.rec_channels, fp_rec);
            fwrite(far, 2, frame_size, fp_far);
            fwrite(out, 2, frame_size * config.out_channels, fp_out);
        }

        fifo_write(re_out, frame_size/2);
    }
    destroy_mux();
    speex_resampler_destroy(rec_rs_state);
    speex_echo_state_destroy(echo_state);
    speex_preprocess_state_destroy(preprocess_state);
    speex_preprocess_state_destroy(den);
    if (fp_far)
    {
        fclose(fp_rec);
        fclose(fp_far);
        fclose(fp_out);
    }

    free(rec);
    free(far);
    free(out);
    free(re_out);

    capture_stop();
    playback_stop();

    exit(0);

    return 0;
}
