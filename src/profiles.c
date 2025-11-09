// profiles.c - simple profile storage using JSON blobs in XDG config

#define _POSIX_C_SOURCE 200809L
#include "profiles.h"
#include "xdg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

static char *profiles_dir(void)
{
    char *xdg = ox_get_xdg_config_home();
    if (!xdg) return NULL;
    // ensure directory exists
    struct stat st;
    if (stat(xdg, &st) != 0) {
        mkdir(xdg, 0700);
    }
    return xdg;
}

int ox_profiles_init(void)
{
    char *d = profiles_dir();
    if (!d) return -1;
    free(d);
    return 0;
}

char *ox_profiles_list_json(void)
{
    char *d = profiles_dir();
    if (!d) return NULL;
    // list files ending with .json using opendir/readdir
    DIR *dir = opendir(d);
    if (!dir) { free(d); return NULL; }
    struct dirent *ent;
    size_t cap = 256; size_t len = 0;
    char *out = malloc(cap);
    if (!out) { closedir(dir); free(d); return NULL; }
    out[0] = '['; len = 1;
    int first = 1;
    while ((ent = readdir(dir)) != NULL) {
        size_t n = strlen(ent->d_name);
        if (n > 5 && strcmp(ent->d_name + n - 5, ".json") == 0) {
            if (!first) { if (len + 2 >= cap) { cap *= 2; out = realloc(out, cap); } out[len++] = ','; }
            else first = 0;
            // append quoted name
            if (len + n + 4 >= cap) { while (len + n + 4 >= cap) cap *= 2; out = realloc(out, cap); }
            out[len++] = '"'; memcpy(out + len, ent->d_name, n); len += n; out[len++] = '"';
        }
    }
    out[len++] = ']'; out[len] = '\0';
    closedir(dir);
    free(d);
    return out;
}

int ox_profiles_save(const char *name, const char *json_blob)
{
    if (!name || !json_blob) return -1;
    char *d = profiles_dir(); if (!d) return -1;
    char path[1024]; snprintf(path, sizeof(path), "%s/%s.json", d, name);
    FILE *f = fopen(path, "w");
    if (!f) { free(d); return -1; }
    fwrite(json_blob, 1, strlen(json_blob), f);
    fclose(f);
    free(d);
    return 0;
}

char *ox_profiles_load(const char *name)
{
    if (!name) return NULL;
    char *d = profiles_dir(); if (!d) return NULL;
    char path[1024]; snprintf(path, sizeof(path), "%s/%s.json", d, name);
    FILE *f = fopen(path, "r"); if (!f) { free(d); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz+1);
    if (!buf) { fclose(f); free(d); return NULL; }
    fread(buf, 1, sz, f); buf[sz] = '\0';
    fclose(f); free(d); return buf;
}

char *ox_profiles_get_vk_token(const char *profile_json)
{
    if (!profile_json) return NULL;
    const char *key = "\"vk_token\":";
    const char *pos = strstr(profile_json, key);
    if (!pos) return NULL;
    pos += strlen(key);
    if (*pos != '"') return NULL;
    pos++;
    const char *end = strchr(pos, '"');
    if (!end) return NULL;
    size_t len = end - pos;
    char *token = malloc(len + 1);
    if (!token) return NULL;
    memcpy(token, pos, len);
    token[len] = '\0';
    return token;
}

char *ox_profiles_set_vk_token(const char *profile_json, const char *token)
{
    if (!profile_json) return NULL;
    char *new_json = NULL;
    if (strstr(profile_json, "\"vk_token\":")) {
        // Replace existing
        const char *start = strstr(profile_json, "\"vk_token\":");
        const char *end = strchr(start, ',');
        if (!end) end = strchr(start, '}');
        if (!end) return NULL;
        size_t prefix_len = start - profile_json;
        size_t suffix_len = strlen(end);
        new_json = malloc(prefix_len + 20 + strlen(token) + suffix_len + 1);
        if (!new_json) return NULL;
        memcpy(new_json, profile_json, prefix_len);
        sprintf(new_json + prefix_len, "\"vk_token\":\"%s\"", token);
        memcpy(new_json + prefix_len + 13 + strlen(token), end, suffix_len + 1);
    } else {
        // Add new
        size_t len = strlen(profile_json);
        if (len < 2) return NULL;
        new_json = malloc(len + 20 + strlen(token) + 1);
        if (!new_json) return NULL;
        memcpy(new_json, profile_json, len - 1);
        sprintf(new_json + len - 1, ",\"vk_token\":\"%s\"}", token);
    }
    return new_json;
}
