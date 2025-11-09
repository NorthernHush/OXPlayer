#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/meta_id3.h"

int main(void)
{
    // Create a fake ID3v2 tag with TIT2 and TPE1 frames for testing
    unsigned char buf[512]; memset(buf, 0, sizeof(buf));
    memcpy(buf, "ID3", 3);
    buf[3] = 3; buf[4] = 0; // v2.3.0
    // syncsafe size: 0x00 00 00 2E (46) -> fits
    buf[6] = 0x00; buf[7] = 0x00; buf[8] = 0x00; buf[9] = 0x2E;
    size_t pos = 10;
    // TIT2 frame (title)
    memcpy(buf + pos, "TIT2", 4); pos += 4;
    // size 0x00000005
    buf[pos++] = 0x00; buf[pos++] = 0x00; buf[pos++] = 0x00; buf[pos++] = 0x05;
    buf[pos++] = 0; buf[pos++] = 0; // flags
    // payload: encoding 0 + "Hey"
    buf[pos++] = 0; memcpy(buf + pos, "Hey", 3); pos += 3;
    // TPE1 frame (artist)
    memcpy(buf + pos, "TPE1", 4); pos += 4;
    buf[pos++] = 0x00; buf[pos++] = 0x00; buf[pos++] = 0x00; buf[pos++] = 0x06;
    buf[pos++] = 0; buf[pos++] = 0;
    buf[pos++] = 0; memcpy(buf + pos, "Me", 2); pos += 2;

    struct ox_metadata m;
    if (ox_meta_id3_parse(buf, sizeof(buf), &m) != 0) {
        fprintf(stderr, "parse failed\n"); return 1;
    }
    printf("title='%s' artist='%s'\n", m.title, m.artist);
    return 0;
}
