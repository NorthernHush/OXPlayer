// audio_pipeline.c - minimal OXXY audio pipeline example
// - uses pcm_ring for inter-thread communication
// - runtime selection for PipeWire (dlopen stub) or ALSA backend
// - dummy backend available when ALSA not requested

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <math.h>
#include "pcm_ring.h"
#include "ui_bridge.h"

#ifdef USE_ALSA
#include <alsa/asoundlib.h>
#endif

#define SAMPLE_RATE 48000
#define CHANNELS OXXY_CHANNELS
#define RING_SECONDS 3

static struct pcm_ring *g_ring = NULL;
static atomic_int g_running = 0;

static int try_init_pipewire(void)
{
    void *h = dlopen("libpipewire-0.3.so", RTLD_NOW | RTLD_LOCAL);
    if (!h) return 0;
    /* Production: implement PipeWire backend here using pw_stream APIs. */
    dlclose(h);
    return 0; /* stub - treat as not available */
}

#ifdef USE_ALSA
struct alsa_ctx { snd_pcm_t *pcm; };

static int alsa_open(struct alsa_ctx *c, unsigned int rate, unsigned int channels)
{
    int rc;
    snd_pcm_hw_params_t *params;
    snd_pcm_open(&c->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(c->pcm, params);
    snd_pcm_hw_params_set_access(c->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(c->pcm, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(c->pcm, params, channels);
    unsigned int r = rate;
    snd_pcm_hw_params_set_rate_near(c->pcm, params, &r, 0);
    snd_pcm_uframes_t frames = 1024;
    int dir = 0;
    snd_pcm_hw_params_set_period_size_near(c->pcm, params, &frames, &dir);
    rc = snd_pcm_hw_params(c->pcm, params);
    snd_pcm_hw_params_free(params);
    return rc;
}

static void alsa_close(struct alsa_ctx *c)
{
    if (c->pcm) { snd_pcm_drain(c->pcm); snd_pcm_close(c->pcm); c->pcm = NULL; }
}
#endif

static void *decoder_thread(void *arg)
{
    (void)arg;
    /* simple sine generator to simulate decoded PCM */
    const double freq = 440.0;
    const double two_pi = 6.283185307179586;
    double phase = 0.0;
    const double inc = two_pi * freq / SAMPLE_RATE;
    const size_t frames_per_chunk = 512;
    float buf[frames_per_chunk * CHANNELS];
    while (atomic_load(&g_running)) {
        for (size_t i = 0; i < frames_per_chunk; ++i) {
            float v = (float)(sin(phase) * 0.2);
            phase += inc; if (phase >= two_pi) phase -= two_pi;
            buf[i*CHANNELS + 0] = v;
            buf[i*CHANNELS + 1] = v;
        }
            /* push peaks to UI bridge (one per frame chunk) */
            float peak = 0.0f;
            for (size_t i = 0; i < frames_per_chunk; ++i) {
                float absv = fabsf(buf[i*CHANNELS]); if (absv > peak) peak = absv;
            }
            ox_ui_push_peak(peak);
        size_t pushed = 0;
        while (pushed < frames_per_chunk && atomic_load(&g_running)) {
            size_t n = pcm_ring_push(g_ring, buf + pushed * CHANNELS, frames_per_chunk - pushed);
            if (n == 0) {
                sleep(0);
                continue;
            }
            pushed += n;
        }
        usleep(1000);
    }
    return NULL;
}

static void *playback_thread(void *arg)
{
    (void)arg;
#ifdef USE_ALSA
    struct alsa_ctx ctx = {0};
    if (alsa_open(&ctx, SAMPLE_RATE, CHANNELS) < 0) {
        fprintf(stderr, "ALSA open failed\n");
        return NULL;
    }
    float local[1024 * CHANNELS];
    while (atomic_load(&g_running)) {
        size_t got = pcm_ring_pop(g_ring, local, 1024);
        if (got == 0) { sleep(0); continue; }
        snd_pcm_sframes_t w = snd_pcm_writei(ctx.pcm, local, got);
        if (w < 0) w = snd_pcm_recover(ctx.pcm, (int)w, 0);
        if (w < 0) { fprintf(stderr, "ALSA write failed\n"); break; }
    }
    alsa_close(&ctx);
#else
    /* Dummy backend: consume from ring and discard (allows running without ALSA) */
    float local[1024 * CHANNELS];
    while (atomic_load(&g_running)) {
        size_t got = pcm_ring_pop(g_ring, local, 1024);
        if (got == 0) { sleep(0); continue; }
        /* simulate writing latency */
        usleep((unsigned int)(1000000 * (double)got / SAMPLE_RATE));
    }
#endif
    return NULL;
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    fprintf(stderr, "OXXY test: starting audio pipeline...\n");
    g_ring = pcm_ring_create(SAMPLE_RATE * RING_SECONDS);
    if (!g_ring) { fprintf(stderr, "failed to create ring\n"); return 1; }

    if (try_init_pipewire()) {
        fprintf(stderr, "PipeWire initialized (not implemented)\n");
    } else {
        fprintf(stderr, "PipeWire not available, using backend as built\n");
    }

    atomic_store(&g_running, 1);
    pthread_t dec, play;
    pthread_create(&dec, NULL, decoder_thread, NULL);
    pthread_create(&play, NULL, playback_thread, NULL);

    fprintf(stderr, "Running for 5 seconds...\n");
    sleep(5);

    atomic_store(&g_running, 0);
    pthread_join(dec, NULL);
    pthread_join(play, NULL);
    pcm_ring_destroy(g_ring);
    fprintf(stderr, "OXXY test: shutdown\n");
    return 0;
}
