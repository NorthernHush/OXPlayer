// vk.c - minimal VK (VKontakte) integration using runtime-detected libcurl via dlopen
#define _POSIX_C_SOURCE 200809L
#include "vk.h"
#include "playlist.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* We attempt to find libcurl and use curl_easy_perform via function pointers.
 * To keep attack surface small we do not follow redirects by default and we enforce timeouts.
 */

typedef void CURL;
typedef size_t (*curl_write_callback_t)(char *, size_t, size_t, void *);

typedef CURL *(*curl_easy_init_t)(void);
typedef int (*curl_easy_setopt_t)(CURL *, int, ...);
typedef int (*curl_easy_perform_t)(CURL *);
typedef void (*curl_easy_cleanup_t)(CURL *);
typedef char *(*curl_easy_strerror_t)(int);

static void *curl_lib = NULL;
static curl_easy_init_t p_curl_easy_init = NULL;
static curl_easy_setopt_t p_curl_easy_setopt = NULL;
static curl_easy_perform_t p_curl_easy_perform = NULL;
static curl_easy_cleanup_t p_curl_easy_cleanup = NULL;
static curl_easy_strerror_t p_curl_easy_strerror = NULL;

struct curl_response {
    char *data;
    size_t size;
};

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    struct curl_response *resp = (struct curl_response *)userdata;
    size_t realsize = size * nmemb;
    char *newdata = realloc(resp->data, resp->size + realsize + 1);
    if (!newdata) return 0;
    resp->data = newdata;
    memcpy(resp->data + resp->size, ptr, realsize);
    resp->size += realsize;
    resp->data[resp->size] = '\0';
    return realsize;
}

int ox_vk_init(void)
{
    curl_lib = dlopen("libcurl.so", RTLD_NOW | RTLD_LOCAL);
    if (!curl_lib) return -1;
    p_curl_easy_init = (curl_easy_init_t)dlsym(curl_lib, "curl_easy_init");
    p_curl_easy_setopt = (curl_easy_setopt_t)dlsym(curl_lib, "curl_easy_setopt");
    p_curl_easy_perform = (curl_easy_perform_t)dlsym(curl_lib, "curl_easy_perform");
    p_curl_easy_cleanup = (curl_easy_cleanup_t)dlsym(curl_lib, "curl_easy_cleanup");
    p_curl_easy_strerror = (curl_easy_strerror_t)dlsym(curl_lib, "curl_easy_strerror");
    if (!p_curl_easy_init || !p_curl_easy_setopt || !p_curl_easy_perform || !p_curl_easy_cleanup) {
        dlclose(curl_lib); curl_lib = NULL; return -1;
    }
    return 0;
}

void ox_vk_shutdown(void)
{
    if (curl_lib) { dlclose(curl_lib); curl_lib = NULL; }
}

int ox_vk_share(const char *access_token, const char *owner_id, const char *message)
{
    if (!curl_lib) return -1;
    (void)access_token; (void)owner_id; (void)message;
    // For brevity, actual curl usage is omitted; returning not-implemented yet.
    return -1;
}

char *ox_vk_oauth(void)
{
    printf("VK OAuth: Visit https://oauth.vk.com/authorize?client_id=YOUR_APP_ID&display=page&redirect_uri=https://oauth.vk.com/blank.html&scope=audio&response_type=token&v=5.131\n");
    printf("After authorization, copy the access_token from the URL and paste it here: ");
    char token[1024];
    if (fgets(token, sizeof(token), stdin) == NULL) return NULL;
    token[strcspn(token, "\n")] = '\0';
    return strdup(token);
}

char *ox_vk_get_audio(const char *access_token)
{
    if (!curl_lib || !access_token) return NULL;
    CURL *curl = p_curl_easy_init();
    if (!curl) return NULL;
    struct curl_response resp = {0};
    char url[2048];
    snprintf(url, sizeof(url), "https://api.vk.com/method/audio.get?access_token=%s&v=5.131", access_token);
    p_curl_easy_setopt(curl, 10002, url); // CURLOPT_URL
    p_curl_easy_setopt(curl, 20011, write_callback); // CURLOPT_WRITEFUNCTION
    p_curl_easy_setopt(curl, 10001, &resp); // CURLOPT_WRITEDATA
    p_curl_easy_setopt(curl, 81, 10L); // CURLOPT_TIMEOUT
    int res = p_curl_easy_perform(curl);
    p_curl_easy_cleanup(curl);
    if (res != 0) {
        free(resp.data);
        return NULL;
    }
    return resp.data;
}

int ox_vk_import_audio_to_playlist(const char *json, struct playlist *p)
{
    if (!json || !p) return -1;
    // Simple JSON parsing: look for "url" fields
    int count = 0;
    const char *pos = json;
    while ((pos = strstr(pos, "\"url\":")) != NULL) {
        pos += 6; // skip "url":
        if (*pos == '"') {
            pos++; // skip "
            const char *end = strchr(pos, '"');
            if (end) {
                char url[2048];
                size_t len = end - pos;
                if (len < sizeof(url)) {
                    memcpy(url, pos, len);
                    url[len] = '\0';
                    playlist_add(p, url);
                    count++;
                }
                pos = end + 1;
            }
        }
    }
    return count;
}
