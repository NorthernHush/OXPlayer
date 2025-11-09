// playlist.c - minimal playlist implementation (.m3u)

#define _POSIX_C_SOURCE 200809L
#include "playlist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct playlist *playlist_create(void)
{
    struct playlist *p = calloc(1, sizeof(*p));
    if (!p) return NULL;
    p->capacity = 16;
    p->items = calloc(p->capacity, sizeof(*p->items));
    return p;
}

void playlist_destroy(struct playlist *p)
{
    if (!p) return;
    for (size_t i = 0; i < p->count; ++i) free(p->items[i].uri);
    free(p->items);
    free(p);
}

int playlist_add(struct playlist *p, const char *uri)
{
    if (!p || !uri) return -1;
    if (p->count == p->capacity) {
        size_t ncap = p->capacity * 2;
        struct playlist_entry *n = realloc(p->items, ncap * sizeof(*n));
        if (!n) return -1;
        p->items = n; p->capacity = ncap;
    }
    p->items[p->count].uri = strdup(uri);
    if (!p->items[p->count].uri) return -1;
    p->count++;
    return 0;
}

int playlist_load_m3u(struct playlist *p, const char *path)
{
    if (!p || !path) return -1;
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char *line = NULL; size_t len = 0;
    while (getline(&line, &len, f) != -1) {
        // strip newline
        size_t l = strlen(line);
        while (l && (line[l-1] == '\n' || line[l-1] == '\r')) { line[--l] = '\0'; }
        if (l == 0) continue;
        if (line[0] == '#') continue;
        playlist_add(p, line);
    }
    free(line); fclose(f);
    return 0;
}

int playlist_save_m3u(struct playlist *p, const char *path)
{
    if (!p || !path) return -1;
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "#EXTM3U\n");
    for (size_t i = 0; i < p->count; ++i) fprintf(f, "%s\n", p->items[i].uri);
    fclose(f);
    return 0;
}

/* Fisher-Yates shuffle */
void playlist_shuffle(struct playlist *p)
{
    if (!p || p->count <= 1) return;
    srand(time(NULL));
    for (size_t i = p->count - 1; i > 0; --i) {
        size_t j = (size_t)rand() % (i + 1);
        char *tmp = p->items[i].uri;
        p->items[i].uri = p->items[j].uri;
        p->items[j].uri = tmp;
    }
}
