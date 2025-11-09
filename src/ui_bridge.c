#define _POSIX_C_SOURCE 200809L
#include "ui_bridge.h"
#include "profiles.h"
#include "vk.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UI_RING_SIZE 4096
static _Atomic size_t ui_head = 0;
static _Atomic size_t ui_tail = 0;
static float ui_data[UI_RING_SIZE];

void ox_ui_push_peak(float peak)
{
    size_t head = atomic_load_explicit(&ui_head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&ui_tail, memory_order_acquire);
    size_t free_count = UI_RING_SIZE - (head - tail);
    if (free_count == 0) return; // drop
    size_t idx = head % UI_RING_SIZE;
    ui_data[idx] = peak;
    atomic_store_explicit(&ui_head, head + 1, memory_order_release);
}

size_t ox_ui_get_waveform_copy(float *dest, size_t max_samples)
{
    size_t head = atomic_load_explicit(&ui_head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&ui_tail, memory_order_relaxed);
    size_t avail = head - tail;
    if (avail == 0) return 0;
    size_t to_copy = avail < max_samples ? avail : max_samples;
    size_t idx = tail % UI_RING_SIZE;
    size_t first = UI_RING_SIZE - idx;
    if (first > to_copy) first = to_copy;
    for (size_t i = 0; i < first; ++i) dest[i] = ui_data[(idx + i) % UI_RING_SIZE];
    if (first < to_copy) {
        for (size_t i = 0; i < to_copy - first; ++i) dest[first + i] = ui_data[i];
    }
    atomic_store_explicit(&ui_tail, tail + to_copy, memory_order_release);
    return to_copy;
}

void ox_ui_request_seek(double seconds) { (void)seconds; }
double ox_ui_get_current_position(void) { return 0.0; }
double ox_ui_get_track_length(void) { return 0.0; }

static struct playlist *global_playlist = NULL;

void ox_ui_set_playlist(struct playlist *p) { global_playlist = p; }

struct playlist *ox_ui_get_playlist(void) { return global_playlist; }

void ox_ui_add_to_playlist(const char *uri) {
    if (global_playlist) playlist_add(global_playlist, uri);
}

/* Profile management from UI */
char *ox_ui_profiles_list_json(void) {
    return ox_profiles_list_json();
}

int ox_ui_profiles_save(const char *name, const char *json_blob) {
    return ox_profiles_save(name, json_blob);
}

char *ox_ui_profiles_load(const char *name) {
    return ox_profiles_load(name);
}

void ox_ui_profiles_free(char *ptr) {
    free(ptr);
}

/* VK import for selected profile */
void ox_ui_vk_import_for_profile(const char *profile_name, struct playlist *p) {
    char *profile_json = ox_profiles_load(profile_name);
    if (!profile_json) return;
    char *token = ox_profiles_get_vk_token(profile_json);
    if (token) {
        char *audio_json = ox_vk_get_audio(token);
        if (audio_json) {
            ox_vk_import_audio_to_playlist(audio_json, p);
            free(audio_json);
        }
        free(token);
    }
    free(profile_json);
}
