// main_launcher.c - launcher for OXXY, starts UI directly
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "profiles.h"
#include "vk.h"
#include "playlist.h"
#include "ui_bridge.h"
#include <stdlib.h>
// No header for audio_pipeline, use extern

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    if (ox_profiles_init() != 0) {
        fprintf(stderr, "Failed to init profiles\n");
        return 1;
    }
    if (ox_vk_init() != 0) {
        fprintf(stderr, "Failed to init VK\n");
        return 1;
    }
    // Create empty playlist for initial UI state
    struct playlist *p = playlist_create();
    if (!p) {
        return 1;
    }
    ox_ui_set_playlist(p);
    // Launch the UI directly - profile selection will happen in UI
    extern int main(int, char**);
    char *args[1] = {NULL};
    main(0, args);
    playlist_destroy(p);
    ox_vk_shutdown();
    return 0;
}
