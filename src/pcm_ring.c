// pcm_ring.c - simple SPSC ring using C11 atomics

#define _POSIX_C_SOURCE 200809L
#include "pcm_ring.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

struct pcm_ring {
    atomic_size_t head; /* write index (frames) */
    atomic_size_t tail; /* read index (frames) */
    size_t capacity;    /* in frames */
    float *data;        /* capacity * channels floats */
};

struct pcm_ring *pcm_ring_create(size_t capacity_frames)
{
    struct pcm_ring *r = calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->capacity = capacity_frames;
    r->data = calloc(capacity_frames * OXXY_CHANNELS, sizeof(float));
    if (!r->data) { free(r); return NULL; }
    atomic_init(&r->head, 0);
    atomic_init(&r->tail, 0);
    return r;
}

void pcm_ring_destroy(struct pcm_ring *r)
{
    if (!r) return;
    free(r->data);
    free(r);
}

size_t pcm_ring_available(const struct pcm_ring *r)
{
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    return head - tail;
}

size_t pcm_ring_free(const struct pcm_ring *r)
{
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    return r->capacity - (head - tail);
}

size_t pcm_ring_push(struct pcm_ring *r, const float *frames, size_t frames_count)
{
    size_t head = atomic_load_explicit(&r->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    size_t free_frames = r->capacity - (head - tail);
    if (free_frames == 0) return 0;
    size_t to_write = frames_count < free_frames ? frames_count : free_frames;

    size_t idx = head % r->capacity;
    size_t first = r->capacity - idx;
    if (first > to_write) first = to_write;
    memcpy(&r->data[idx * OXXY_CHANNELS], frames, first * OXXY_CHANNELS * sizeof(float));
    if (first < to_write) {
        memcpy(&r->data[0], frames + first * OXXY_CHANNELS, (to_write - first) * OXXY_CHANNELS * sizeof(float));
    }
    atomic_store_explicit(&r->head, head + to_write, memory_order_release);
    return to_write;
}

size_t pcm_ring_pop(struct pcm_ring *r, float *out_frames, size_t frames_count)
{
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_relaxed);
    size_t avail = head - tail;
    if (avail == 0) return 0;
    size_t to_read = frames_count < avail ? frames_count : avail;

    size_t idx = tail % r->capacity;
    size_t first = r->capacity - idx;
    if (first > to_read) first = to_read;
    memcpy(out_frames, &r->data[idx * OXXY_CHANNELS], first * OXXY_CHANNELS * sizeof(float));
    if (first < to_read) {
        memcpy(out_frames + first * OXXY_CHANNELS, &r->data[0], (to_read - first) * OXXY_CHANNELS * sizeof(float));
    }
    atomic_store_explicit(&r->tail, tail + to_read, memory_order_release);
    return to_read;
}
