#pragma once

#include <stddef.h>

#define OXXY_CHANNELS 2

struct pcm_ring;

/* Create a ring buffer that can hold capacity_frames frames (each frame has OXXY_CHANNELS floats).
 * Returns NULL on allocation failure.
 */
struct pcm_ring *pcm_ring_create(size_t capacity_frames);
void pcm_ring_destroy(struct pcm_ring *r);

/* Push up to frames_count frames into ring. Returns frames pushed. */
size_t pcm_ring_push(struct pcm_ring *r, const float *frames, size_t frames_count);

/* Pop up to frames_count frames from ring into out_frames. Returns frames popped. */
size_t pcm_ring_pop(struct pcm_ring *r, float *out_frames, size_t frames_count);

/* Get approximate available frames to read */
size_t pcm_ring_available(const struct pcm_ring *r);

/* Get approximate free frames for writing */
size_t pcm_ring_free(const struct pcm_ring *r);
