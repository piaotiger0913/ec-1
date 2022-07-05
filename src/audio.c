// audio.c

#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <error.h>
#include <sys/stat.h>

#include <alsa/asoundlib.h>

#include "pa_ringbuffer.h"
#include "audio.h"
#include "conf.h"
#include "util.h"

#include <speex/speex_resampler.h>

pthread_mutex_t thr;
pthread_cond_t cond;

PaUtilRingBuffer g_playback_ringbuffer;
PaUtilRingBuffer g_capture_ringbuffer;

static pthread_t g_playback_thread;
static pthread_t g_capture_thread;

extern int g_is_quit;

int pFlag = 0;
int Pcount = 1;
int P1count = 1;

// #define MIN(x, y) ((x) < (y) ? (x) : (y))

// static float max_amplitude;

static int xrun_recovery(snd_pcm_t *handle, int err)
{

    if (err == -EPIPE)
    { /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            fprintf(stderr, "Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
    }
    else if (err == -ESTRPIPE)
    {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(0.01); /* wait until the suspend flag is released */
        if (err < 0)
        {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                fprintf(stderr, "Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
    }
    return err;
}

int set_params(snd_pcm_t *handle, snd_pcm_hw_params_t *hw_params, unsigned rate, unsigned channels, unsigned chunk_size)
{
    int err;
    int mmap = 0;

    err = snd_pcm_hw_params_malloc(&hw_params);
    assert(err >= 0);

    err = snd_pcm_hw_params_any(handle, hw_params);
    assert(err >= 0);

    // mmap
    if (snd_pcm_hw_params_test_access(handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED) >= 0)
    {
        mmap = 1;
        err = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
    }
    else
    {
        err = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    }
    assert(err >= 0);

    err = snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE);
    assert(err >= 0);

    err = snd_pcm_hw_params_set_rate(handle, hw_params, rate, 0);
    assert(err >= 0);

    err = snd_pcm_hw_params_set_channels(handle, hw_params, channels);
    assert(err >= 0);

    err = snd_pcm_hw_params_set_buffer_size(handle, hw_params, chunk_size * 2);
    assert(err >= 0);

    // No supported by PulseAudio's ALSA plugin
    // err = snd_pcm_hw_params_set_period_size(handle, hw_params, chunk_size, 0);
    // assert(err >= 0);

    err = snd_pcm_hw_params(handle, hw_params);
    if (err < 0)
    {
        fprintf(stderr, "Unable to install hw params:");
        exit(1);
    }

    // {
    //     snd_output_t *out;
    //     snd_output_stdio_attach(&out, stderr, 0);
    //     snd_pcm_hw_params_dump(hw_params, out);
    //     snd_output_close(out);
    // }

    return mmap;
}

void *playback(void *ptr)
{
    snd_pcm_hw_params_t *hw_params = NULL;
    int err;
    unsigned chunk_bytes;
    unsigned frame_bytes;
    char *chunk = NULL;
    snd_pcm_t *handle;
    unsigned chunk_size = 1024;
    unsigned zero_count = 0;
    conf_t *conf = (conf_t *)ptr;
    int mmap = 0;

    char *outBuf = NULL;

    if ((err = snd_pcm_open(&handle, conf->out_pcm, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        fprintf(stderr, "cannot open audio device %s (%s)\n",
                conf->out_pcm,
                snd_strerror(err));
        exit(1);
    }

    mmap = set_params(handle, hw_params, conf->rate, conf->ref_channels, chunk_size);

    frame_bytes = conf->ref_channels * 2;
    chunk_bytes = chunk_size * frame_bytes;
    chunk = (char *)malloc(chunk_bytes);
    if (chunk == NULL)
    {
        fprintf(stderr, "not enough memory\n");
        exit(1);
    }

    struct stat st;

    if (stat(conf->playback_fifo, &st) != 0)
    {
        mkfifo(conf->playback_fifo, 0666);
    }
    else if (!S_ISFIFO(st.st_mode))
    {
        remove(conf->playback_fifo);
        mkfifo(conf->playback_fifo, 0666);
    }

    int fd = open(conf->playback_fifo, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open %s, error %d\n", conf->playback_fifo, fd);
        exit(1);
    }
    long pipe_size = (long)fcntl(fd, F_GETPIPE_SZ);
    if (pipe_size == -1)
    {
        perror("get pipe size failed.");
    }
    printf("default pipe size: %ld\n", pipe_size);

    int ret = fcntl(fd, F_SETPIPE_SZ, chunk_bytes * 4);
    if (ret < 0)
    {
        perror("set pipe size failed.");
    }

    pipe_size = (long)fcntl(fd, F_GETPIPE_SZ);
    if (pipe_size == -1)
    {
        perror("get pipe size 2 failed.");
    }
    printf("new pipe size: %ld\n", pipe_size);

    int wait_us = chunk_size * 1000000 / conf->rate / 4;
    //resample init 
    int resampler_err = 0;
    SpeexResamplerState *play_rs_state = 
		speex_resampler_init(1,//infile_info.channels,
							 8000,//infile_info.samplerate,
							 conf->rate,
							 10,
							 &resampler_err);

	speex_resampler_skip_zeros(play_rs_state);
    outBuf = (char *)malloc(chunk_bytes * 2);
    
    while (!g_is_quit)
    {
        int count = 0;

        for (int i = 0; i < 2; i++)
        {
            int result = read(fd, chunk + count, chunk_bytes - count);

            uint32_t in_pro = result;
            uint32_t out_pro = result;

            err = speex_resampler_process_int(play_rs_state, 0, chunk + count, &in_pro, outBuf, &out_pro);
            
            
            if (result < 0)
            {
                if (errno != EAGAIN)
                {
                    fprintf(stderr, "read() returned %d, errno = %d\n", result, errno);
                    exit(1);
                }
            }
            else
            {
                count += result;
                // printf("count  %d bytes zero\n", count);
            }

            if (count >= chunk_bytes)
            {
                break;
            }

            usleep(wait_us);
        }

        if (count < chunk_bytes)
        {
            memset(chunk + count, 0, chunk_bytes - count);
            memset(outBuf, 0, chunk_bytes * 2);
            

            if (count)
            {
                printf("playback filled %d bytes zero\n", chunk_bytes - count);
            }
        }

        if (0 == count)
        {
            // bypass AEC when no playback
            if (zero_count > (conf->filter_length + conf->buffer_size))
            {
                if (!conf->bypass)
                {
                    conf->bypass = 1;
                    printf("No playback, bypass AEC\n");
                }
            }
            else
            {
                zero_count += chunk_size;
            }
        }
        else
        {
            if (conf->bypass)
            {
                conf->bypass = 0;
                zero_count = 0;
                printf("Enable AEC\n");
            }
        }

        count = chunk_size *2;
        char *data;
        // if (conf->bypass){
        //     memset(outBuf, 0, chunk_bytes * 2);
        // }
        data = (char *)outBuf;
        while (count > 0 && !g_is_quit)
        {

            ssize_t r;
            if (mmap)
            {
                r = snd_pcm_mmap_writei(handle, data, count);
            }
            else
            {
                r = snd_pcm_writei(handle, data, count);
            }

            if (r == -EAGAIN || (r >= 0 && (size_t)r < count))
            {
                fprintf(stderr, "w playback read error: %s\n", snd_strerror(r));
                snd_pcm_wait(handle, 100);
            }
            else if (r < 0)
            {
                fprintf(stderr, "playback read error: %s\n", snd_strerror(r));
                if (xrun_recovery(handle, r) < 0)
                {
                    exit(1);
                }
            }
            if (r > 0)
            {
                pthread_mutex_lock(&thr);
                PaUtil_WriteRingBuffer(&g_playback_ringbuffer, data, r);

                count -= r;
                data += r * frame_bytes;
                
                // while(Pcount % 2 != 0) {

                //     pthread_cond_wait(&cond, &thr);

                // }
                // // // printf("%d ", Pcount++);
                // Pcount++;
                // printf("%d ", P1count++);

                pthread_mutex_unlock(&thr);
                // pthread_cond_signal(&cond);
            }
        }
    }
    
    snd_pcm_close(handle);

    speex_resampler_destroy(play_rs_state);
    
    free(chunk);
    free(outBuf);

    return NULL;
}

void *capture(void *ptr)
{
    snd_pcm_hw_params_t *hw_params = NULL;
    int err;
    unsigned frame_bytes;
    void *chunk = NULL;
    snd_pcm_t *handle;
    unsigned chunk_size = 1024;
    conf_t *conf = (conf_t *)ptr;
    int mmap = 0;

    if ((err = snd_pcm_open(&handle, conf->rec_pcm, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        fprintf(stderr, "cannot open audio device %s (%s)\n",
                conf->rec_pcm,
                snd_strerror(err));
        exit(1);
    }

    mmap = set_params(handle, hw_params, conf->rate, conf->rec_channels, chunk_size * 2);

    frame_bytes = conf->rec_channels * 2;
    chunk = malloc(chunk_size * frame_bytes);
    if (chunk == NULL)
    {
        fprintf(stderr, "not enough memory\n");
        exit(1);
    }

    while (!g_is_quit)
    {

        ssize_t r;
        if (mmap)
        {
            r = snd_pcm_mmap_readi(handle, chunk, chunk_size);
        }
        else
        {
            r = snd_pcm_readi(handle, chunk, chunk_size);
        }
        if (r == -EAGAIN || (r >= 0 && (size_t)r < chunk_size))
        {
            fprintf(stderr, "1 read error: %s\n", snd_strerror(r));
            snd_pcm_wait(handle, 100);
        }
        else if (r < 0)
        {
            fprintf(stderr, "read error: %s\n", snd_strerror(r));
            if (xrun_recovery(handle, r) < 0)
            {
                exit(1);
            }
        }

        if (r > 0)
        {
            pthread_mutex_lock(&thr);

            ring_buffer_size_t written =
                PaUtil_WriteRingBuffer(&g_capture_ringbuffer, chunk, r);

            if (written < (r))
            {
                printf("lost capture %ld frames\n", r - written);
            }

            // while(Pcount % 2 != 1) {

            //     pthread_cond_wait(&cond, &thr);

            // }
            // // // printf("%d ", Pcount++);
            // Pcount++;

            // // printf("%d ", Pcount++);
            pthread_mutex_unlock(&thr);
            // pthread_cond_signal(&cond);
        }
    }

    snd_pcm_close(handle);
    free(chunk);

    return NULL;
}

int capture_start(conf_t *conf)
{
    unsigned buffer_size = power2(conf->buffer_size);
    unsigned buffer_bytes = conf->rec_channels * conf->bits_per_sample / 8;

    void *buf = calloc(buffer_size, buffer_bytes);
    if (buf == NULL)
    {
        fprintf(stderr, "Fail to allocate memory.\n");
        exit(1);
    }

    ring_buffer_size_t ret = PaUtil_InitializeRingBuffer(&g_capture_ringbuffer, buffer_bytes, buffer_size, buf);
    if (ret == -1)
    {
        fprintf(stderr, "Initialize ring buffer but element count is not a power of 2.\n");
        exit(1);
    }

    pthread_create(&g_capture_thread, NULL, capture, conf);

    return 0;
}

int playback_start(conf_t *conf)
{
    unsigned buffer_size = power2(conf->buffer_size);
    unsigned buffer_bytes = conf->ref_channels * conf->bits_per_sample / 8;

    void *buf = calloc(buffer_size, buffer_bytes);
    if (buf == NULL)
    {
        fprintf(stderr, "Fail to allocate memory.\n");
        exit(1);
    }

    ring_buffer_size_t ret = PaUtil_InitializeRingBuffer(&g_playback_ringbuffer, buffer_bytes, buffer_size, buf);
    if (ret == -1)
    {
        fprintf(stderr, "Initialize ring buffer but element count is not a power of 2.\n");
        exit(1);
    }
    pthread_create(&g_playback_thread, NULL, playback, conf);

    return 0;
}
void create_mux()
{
    pthread_mutex_init(&thr, 0);
    pthread_cond_init(&cond, 0);
}
void destroy_mux()
{
    pthread_mutex_destroy(&thr);
    pthread_cond_destroy(&cond);
}
int capture_stop()
{
    void *ret = NULL;
    pthread_join(g_capture_thread, &ret);

    free(g_capture_ringbuffer.buffer);

    return 0;
}

int playback_stop()
{
    void *ret = NULL;
    pthread_join(g_playback_thread, &ret);

    free(g_playback_ringbuffer.buffer);

    return 0;
}

int capture_read(void *buf, size_t frames, int timeout_ms)
{
    while (PaUtil_GetRingBufferReadAvailable(&g_capture_ringbuffer) < frames && timeout_ms > 0)
    {
        usleep(10);
        timeout_ms--;
    }

    return PaUtil_ReadRingBuffer(&g_capture_ringbuffer, buf, frames);
}

int capture_skip(size_t frames)
{
    // int16_t *buf1;
    while (PaUtil_GetRingBufferReadAvailable(&g_capture_ringbuffer) < frames)
    {
        usleep(1000);
    }

    return PaUtil_AdvanceRingBufferReadIndex(&g_capture_ringbuffer, frames);
    // PaUtil_ReadRingBuffer(&g_capture_ringbuffer, buf1, frames);
    // for (int i=0; i<frames;i++){
    //     printf("error %d\n",buf1[i]);
    // }
    // return 0;
}

int playback_read(void *buf, size_t frames, int timeout_ms)
{
    while (PaUtil_GetRingBufferReadAvailable(&g_playback_ringbuffer) < frames && timeout_ms > 0)
    {
        usleep(1000);
        timeout_ms--;
    }

    return PaUtil_ReadRingBuffer(&g_playback_ringbuffer, buf, frames);
}

