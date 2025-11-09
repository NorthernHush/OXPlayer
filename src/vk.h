#pragma once
#include <stddef.h>
#include "playlist.h"

/* Minimal VK integration API for OXXY.
 * This module will attempt to detect libcurl at runtime and perform requests to VK API.
 * If libcurl is not available, functions will return an error code.
 */

int ox_vk_init(void);
void ox_vk_shutdown(void);

/* Share a message (e.g., current track) to VK wall using provided access_token and owner_id.
 * Returns 0 on success, -1 on failure.
 */
int ox_vk_share(const char *access_token, const char *owner_id, const char *message);

/* Perform OAuth flow: prompt user for token URL, return access_token on success.
 * Returns malloc'd token string or NULL on failure.
 */
char *ox_vk_oauth(void);

/* Fetch user's audio list using access_token.
 * Returns malloc'd JSON string or NULL on failure.
 */
char *ox_vk_get_audio(const char *access_token);

/* Parse audio JSON and add URLs to playlist.
 * Returns number of tracks added or -1 on failure.
 */
int ox_vk_import_audio_to_playlist(const char *json, struct playlist *p);
