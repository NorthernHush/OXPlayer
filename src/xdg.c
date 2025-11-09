// xdg.c - simple XDG path helpers

#define _POSIX_C_SOURCE 200809L
#include "xdg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *ox_get_xdg_config_home(void)
{
    const char *p = getenv("XDG_CONFIG_HOME");
    if (!p || p[0] == '\0') {
        const char *home = getenv("HOME");
        if (!home) return NULL;
        size_t n = strlen(home) + strlen("/.config/oxxy") + 1;
        char *out = malloc(n);
        if (!out) return NULL;
        snprintf(out, n, "%s/.config/oxxy", home);
        return out;
    }
    size_t n = strlen(p) + strlen("/oxxy") + 1;
    char *out = malloc(n);
    if (!out) return NULL;
    snprintf(out, n, "%s/oxxy", p);
    return out;
}

char *ox_get_xdg_cache_home(void)
{
    const char *p = getenv("XDG_CACHE_HOME");
    if (!p || p[0] == '\0') {
        const char *home = getenv("HOME");
        if (!home) return NULL;
        size_t n = strlen(home) + strlen("/.cache/oxxy") + 1;
        char *out = malloc(n);
        if (!out) return NULL;
        snprintf(out, n, "%s/.cache/oxxy", home);
        return out;
    }
    size_t n = strlen(p) + strlen("/oxxy") + 1;
    char *out = malloc(n);
    if (!out) return NULL;
    snprintf(out, n, "%s/oxxy", p);
    return out;
}
