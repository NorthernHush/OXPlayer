// meta_id3.h - minimal ID3v2 tag parser (subset) for OXXY
#pragma once

#include <stddef.h>

struct ox_metadata {
    char title[256];
    char artist[256];
    char album[256];
    int year;
    int track;
    int duration_sec; /* optional */
};

/* Parse ID3v2 tag from memory buffer (buf,len). Returns 0 on success, -1 on error.
 * This is a minimal, robust parser: it stops on malformed data and bounds-checks.
 */
int ox_meta_id3_parse(const unsigned char *buf, size_t len, struct ox_metadata *out);
