// ui/ui_main.cpp
// Custom GLFW + OpenGL3 neon-themed UI for OXXY (no Dear ImGui dependency)
// Features:
// - Neon/cyberpunk color scheme
// - Waveform visualization (synthetic waveform for demo)
// - Album art crossfade placeholder
// - Simple scrubber and clickable Play/Pause button

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>

// Externs for UI bridge
extern "C" {
#include "ui_bridge.h"
}

// Minimal helper: draw colored rectangle using immediate mode via glDrawArrays
// For simplicity we use a very small compatibility helper (works with GL3 core if VAO/VBO setup done)

static void draw_rect(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_line_strip(const std::vector<float> &xs, const std::vector<float> &ys, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < xs.size(); ++i) glVertex2f(xs[i], ys[i]);
    glEnd();
}

// Simple mapping helpers for NDC coordinate drawing in window coords
static int win_w = 1280, win_h = 720;
static void ortho_setup()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, win_w, win_h, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    if (!glfwInit()) { fprintf(stderr, "GLFW init failed\n"); return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow *w = glfwCreateWindow(win_w, win_h, "OXXY — Neon UI", NULL, NULL);
    if (!w) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);

    // UI state
    bool playing = false;
    double progress = 0.0, length = 180.0;
    float volume = 0.8f;
    bool show_login = false;
    bool show_add_music = false;
    char input_text[256] = {0};
    int input_cursor = 0;

    // Waveform buffer (filled from audio thread via ui_bridge)
    const size_t samples = 2048;
    std::vector<float> wave(samples);

    auto last = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        glfwGetFramebufferSize(w, &win_w, &win_h);
        glViewport(0, 0, win_w, win_h);
        ortho_setup();
        glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Neon accent
        float nr = 0.0f/255.0f, ng = 180.0f/255.0f, nb = 255.0f/255.0f;

        // Draw top bar
        draw_rect(10, 10, win_w - 20, 80, 0.06f, 0.07f, 0.09f, 1.0f);
        // Draw album art placeholder
        draw_rect(30, 30, 64, 64, 0.06f, 0.07f, 0.09f, 1.0f);
        // Draw waveform area
        const float wfx = 110, wfy = 120; const float wfw = win_w - 140, wfh = 160;
        draw_rect(wfx - 4, wfy - 4, wfw + 8, wfh + 8, 0.02f, 0.02f, 0.03f, 1.0f);

        // Build line points
        std::vector<float> xs, ys;
        xs.reserve(samples); ys.reserve(samples);
        // copy available peaks from bridge into wave (non-blocking)
        extern size_t ox_ui_get_waveform_copy(float *, size_t);
        static float scratch[2048];
        size_t copied = ox_ui_get_waveform_copy(scratch, samples);
        if (copied > 0) {
            for (size_t i = 0; i < copied && i < samples; ++i) wave[i] = scratch[i];
        }
        for (size_t i = 0; i < samples; ++i) {
            float x = wfx + (float)i / (samples - 1) * wfw;
            float y = wfy + wfh * 0.5f * (1.0f - wave[i]);
            xs.push_back(x); ys.push_back(y);
        }
        draw_line_strip(xs, ys, nr, ng, nb, 0.9f);

        // Draw EQ bars
        for (int i = 0; i < 12; ++i) {
            float bx = 40.0f + i * 22.0f;
            float bh = 20.0f + 80.0f * fabs(sin((float)glfwGetTime() * (0.3f + i * 0.05f)));
            draw_rect(bx, win_h - 140 - bh, 16, bh, nr, ng, nb, 1.0f);
        }

        // Draw top menu
        draw_rect(10, 10, win_w - 20, 40, 0.06f, 0.07f, 0.09f, 1.0f);
        // VK Login button
        draw_rect(20, 15, 100, 30, nr, ng, nb, show_login ? 1.0f : 0.6f);
        // Telegram Login button (placeholder)
        draw_rect(130, 15, 120, 30, nr, ng, nb, 0.6f);
        // Add Music button
        draw_rect(260, 15, 100, 30, nr, ng, nb, show_add_music ? 1.0f : 0.6f);

        // Draw playback controls
        float btnw = 80, btnh = 40;
        float bx = win_w * 0.5f - 1.5f * (btnw + 10);
        float by = win_h - 80;
        // Play/Pause
        draw_rect(bx, by, btnw, btnh, nr, ng, nb, playing ? 1.0f : 0.6f);
        // Next
        draw_rect(bx + (btnw + 10), by, btnw, btnh, nr, ng, nb, 0.6f);
        // Prev
        draw_rect(bx + 2*(btnw + 10), by, btnw, btnh, nr, ng, nb, 0.6f);

        // Scrubber
        float sbx = 50, sby = win_h - 140; float sbw = win_w - 100, sbh = 10;
        draw_rect(sbx, sby, sbw, sbh, 0.08f, 0.09f, 0.11f, 1.0f);
        float fill = (float)(progress / length);
        if (fill < 0.0f) fill = 0.0f; if (fill > 1.0f) fill = 1.0f;
        draw_rect(sbx, sby, sbw * fill, sbh, nr, ng, nb, 1.0f);

        // Mouse interaction (simple)
        double mx, my; int ml = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT);
        glfwGetCursorPos(w, &mx, &my);
        if (ml == GLFW_PRESS) {
            // VK Login button
            if (mx >= 20 && mx <= 120 && my >= 15 && my <= 45) {
                show_login = !show_login;
                show_add_music = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            // Telegram Login button (placeholder)
            if (mx >= 130 && mx <= 250 && my >= 15 && my <= 45) {
                // Placeholder for Telegram login
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            // Add Music button
            if (mx >= 260 && mx <= 360 && my >= 15 && my <= 45) {
                show_add_music = !show_add_music;
                show_login = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            // Add button in panel
            if (show_add_music && mx >= 60 && mx <= 160 && my >= 120 && my <= 150) {
                if (strlen(input_text) > 0) {
                    ox_ui_add_to_playlist(input_text);
                    input_text[0] = '\0';
                    input_cursor = 0;
                    show_add_music = false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            // if click in scrubber area, update progress
            if (mx >= sbx && mx <= sbx + sbw && my >= sby && my <= sby + sbh) {
                progress = (mx - sbx) / sbw * length;
            }
            // play/pause button
            if (mx >= bx && mx <= bx + btnw && my >= by && my <= by + btnh) {
                playing = !playing;
                // simple debounce
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
        }

        // Draw login panel if active
        if (show_login) {
            draw_rect(50, 60, 400, 200, 0.1f, 0.1f, 0.12f, 1.0f);
            // Placeholder text
        }

        // Draw add music panel if active
        if (show_add_music) {
            draw_rect(50, 60, 400, 200, 0.1f, 0.1f, 0.12f, 1.0f);
            // Input field for file path
            draw_rect(60, 80, 380, 30, 0.05f, 0.05f, 0.07f, 1.0f);
            // Add button
            draw_rect(60, 120, 100, 30, nr, ng, nb, 0.6f);
        }

        // Update demo waveform animation
        for (size_t i = 0; i < samples; ++i) wave[i] = sin((double)i / samples * 20.0 + glfwGetTime() * 2.0) * (0.3 + 0.15 * sin(i * 0.03 + glfwGetTime()));

        // Advance progress if playing
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last).count();
        last = now;
        if (playing) progress += dt;
        if (progress >= length) { progress = 0.0; }

        glfwSwapBuffers(w);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}
