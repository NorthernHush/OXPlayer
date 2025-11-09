#include <stdio.h>
#include <stdlib.h>
#include "../src/playlist.h"

int main(void)
{
    struct playlist *p = playlist_create();
    if (!p) return 1;
    playlist_add(p, "/tmp/song1.mp3");
    playlist_add(p, "/tmp/song2.mp3");
    playlist_save_m3u(p, "/tmp/test.m3u");
    playlist_destroy(p);
    printf("playlist test wrote /tmp/test.m3u\n");
    return 0;
}
