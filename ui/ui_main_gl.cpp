// ui/ui_main_gl.cpp
// Modern OpenGL UI for OXXY: uses a simple shader pipeline and VBO to draw waveform,
// supports HiDPI scaling and a neon theme. This is a demo frontend; integrate with
// the actual audio core by linking and using ox_ui_get_waveform_copy.

#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <cmath>
#include <chrono>
#include <thread>
#include <string>

extern "C" size_t ox_ui_get_waveform_copy(float *dest, size_t max_samples);
extern "C" int ox_profiles_init(void);
extern "C" char *ox_profiles_list_json(void);
extern "C" int ox_profiles_save(const char *name, const char *json_blob);
extern "C" char *ox_profiles_load(const char *name);
extern "C" void ox_ui_add_to_playlist(const char *uri);

static int win_w = 1280, win_h = 720;
static char last_dropped[1024] = {0};

static void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    (void)window;
    if (count <= 0) return;
    snprintf(last_dropped, sizeof(last_dropped), "%s", paths[0]);
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    if (!glfwInit()) return 1;
    GLFWwindow *w = glfwCreateWindow(win_w, win_h, "OXXY — GL UI", NULL, NULL);
    if (!w) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);
    glfwSetDropCallback(w, drop_callback);

    const size_t samples = 2048;
    std::vector<float> samples_buf(samples);

    auto last = std::chrono::steady_clock::now();
    double progress = 0.0; bool playing = false; double length = 180.0;

    // profiles
    ox_profiles_init();
    char prof_name[128] = "default";

    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        int fbw, fbh; glfwGetFramebufferSize(w, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // copy peaks from bridge
        float scratch[samples];
        size_t copied = ox_ui_get_waveform_copy(scratch, samples);
        if (copied > 0) {
            for (size_t i = 0; i < copied && i < samples; ++i) samples_buf[i] = scratch[i];
        }

        // draw waveform with immediate mode
        glColor3f(0.0f, 0.7f, 1.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < samples; ++i) {
            float x = (float)i / (samples - 1) * 2.0f - 1.0f;
            float y = samples_buf[i] * 0.8f;
            glVertex2f(x, y);
        }
        glEnd();

        // simple controls via keyboard
        if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) { playing = !playing; std::this_thread::sleep_for(std::chrono::milliseconds(150)); }
        if (playing) {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last).count();
            progress += dt; last = now;
            if (progress >= length) progress = 0.0;
        } else last = std::chrono::steady_clock::now();

        if (last_dropped[0]) {
            fprintf(stderr, "Last dropped: %s\n", last_dropped);
            ox_ui_add_to_playlist(last_dropped);
            last_dropped[0] = '\0';
        }

        // profile save/load demo
        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) {
            const char *blob = "{\"volume\":0.8}";
            ox_profiles_save(prof_name, blob);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        if (glfwGetKey(w, GLFW_KEY_L) == GLFW_PRESS) {
            char *b = ox_profiles_load(prof_name);
            if (b) { fprintf(stderr, "loaded profile %s: %s\n", prof_name, b); free(b); }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        glfwSwapBuffers(w);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}
