// meta_id3.c - minimal safe ID3v2 parser (supports TIT2, TPE1, TALB basic frames)

#define _POSIX_C_SOURCE 200809L
#include "meta_id3.h"
#include <string.h>
#include <stdio.h>

static unsigned int syncsafe_to_size(const unsigned char s[4])
{
    /* ID3v2 syncsafe integer: 4 * 7 bits */
    return (s[0] << 21) | (s[1] << 14) | (s[2] << 7) | (s[3]);
}

int ox_meta_id3_parse(const unsigned char *buf, size_t len, struct ox_metadata *out)
{
    if (!buf || !out || len < 10) return -1;
    if (memcmp(buf, "ID3", 3) != 0) return -1;

    unsigned int tag_size = syncsafe_to_size(&buf[6]);
    if (tag_size + 10 > len) tag_size = (unsigned int)(len - 10);

    memset(out, 0, sizeof(*out));

    size_t pos = 10; /* start of frames */
    while (pos + 10 <= 10 + tag_size) {
        const unsigned char *f = buf + pos;
        char frame_id[5] = {0};
        memcpy(frame_id, f, 4);
        unsigned int frame_size = (f[4] << 24) | (f[5] << 16) | (f[6] << 8) | (f[7]);

        if (frame_size == 0) break;
        if (pos + 10 + frame_size > 10 + tag_size) break; /* malformed */
        const unsigned char *pdata = f + 10;
        if (strcmp(frame_id, "TIT2") == 0) {
            /* text encoding byte then utf-8/iso-8859-1 text */
            if (frame_size >= 1) {
                size_t sl = frame_size - 1;
                size_t copy = sl < sizeof(out->title)-1 ? sl : sizeof(out->title)-1;
                memcpy(out->title, pdata+1, copy);
                out->title[copy] = '\0';
            }
        } else if (strcmp(frame_id, "TPE1") == 0) {
            if (frame_size >= 1) {
                size_t sl = frame_size - 1;
                size_t copy = sl < sizeof(out->artist)-1 ? sl : sizeof(out->artist)-1;
                memcpy(out->artist, pdata+1, copy);
                out->artist[copy] = '\0';
            }
        } else if (strcmp(frame_id, "TALB") == 0) {
            if (frame_size >= 1) {
                size_t sl = frame_size - 1;
                size_t copy = sl < sizeof(out->album)-1 ? sl : sizeof(out->album)-1;
                memcpy(out->album, pdata+1, copy);
                out->album[copy] = '\0';
            }
        }
        pos += 10 + frame_size;
    }
    return 0;
}
