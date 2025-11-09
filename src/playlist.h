#pragma once
#include <stddef.h>

struct playlist_entry { char *uri; };

struct playlist {
    struct playlist_entry *items;
    size_t count;
    size_t capacity;
    size_t pos;
    int shuffle; /* 0/1 */
    int repeat; /* 0: none, 1: all, 2: one */
};

struct playlist *playlist_create(void);
void playlist_destroy(struct playlist *p);
int playlist_add(struct playlist *p, const char *uri);
int playlist_load_m3u(struct playlist *p, const char *path);
int playlist_save_m3u(struct playlist *p, const char *path);
void playlist_shuffle(struct playlist *p);
