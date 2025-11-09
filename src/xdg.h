#pragma once
#include <stddef.h>

/* Return malloc'd string with XDG_CONFIG_HOME path (ends without slash).
 * Caller must free. Falls back to $HOME/.config
 */
char *ox_get_xdg_config_home(void);

/* Return malloc'd string with XDG_CACHE_HOME path. Caller must free. */
char *ox_get_xdg_cache_home(void);
