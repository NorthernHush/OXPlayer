#pragma once
#include <stddef.h>

int ox_profiles_init(void);
char *ox_profiles_list_json(void); /* returns malloc'd JSON string */
int ox_profiles_save(const char *name, const char *json_blob);
char *ox_profiles_load(const char *name); /* returns malloc'd JSON blob */

/* Helper to get VK token from profile JSON */
char *ox_profiles_get_vk_token(const char *profile_json);

/* Helper to set VK token in profile JSON */
char *ox_profiles_set_vk_token(const char *profile_json, const char *token);
