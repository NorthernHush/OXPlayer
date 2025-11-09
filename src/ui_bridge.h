// ui_bridge.h - bridge between audio core and UI (waveform + controls + profiles)
#pragma once
#include <stddef.h>
#include "playlist.h"

#ifdef __cplusplus
extern "C" {
#endif

/* push a peak value (range -1..1) from audio thread into a small ring for UI */
void ox_ui_push_peak(float peak);

/* copy up to max_samples into dest, returns number of samples copied */
size_t ox_ui_get_waveform_copy(float *dest, size_t max_samples);

/* UI -> audio control requests */
void ox_ui_request_seek(double seconds);
double ox_ui_get_current_position(void);
double ox_ui_get_track_length(void);

/* Playlist management from UI */
void ox_ui_set_playlist(struct playlist *p);
struct playlist *ox_ui_get_playlist(void);
void ox_ui_add_to_playlist(const char *uri);

/* Profile management from UI */
char *ox_ui_profiles_list_json(void);
int ox_ui_profiles_save(const char *name, const char *json_blob);
char *ox_ui_profiles_load(const char *name);
void ox_ui_profiles_free(char *ptr);

/* VK import for selected profile */
void ox_ui_vk_import_for_profile(const char *profile_name, struct playlist *p);

#ifdef __cplusplus
}
#endif
