#define _POSIX_C_SOURCE 199309L

#include "core/timing.h"
#include "utils/logger.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
    #include <unistd.h>
#endif

/* Timing state */
static struct {
    double start_time;
    double frame_start;
    double last_frame;
    double delta_time;
    double fps;
    int frame_count;
    double fps_timer;
} g_timing = {0};

/* Platform-specific time functions */
#ifdef _WIN32
static double get_time_seconds(void) {
    static LARGE_INTEGER frequency;
    static bool initialized = false;

    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}
#else
static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif

void timing_init(void) {
    g_timing.start_time = get_time_seconds();
    g_timing.frame_start = g_timing.start_time;
    g_timing.last_frame = g_timing.start_time;
    g_timing.delta_time = 0.0;
    g_timing.fps = 0.0;
    g_timing.frame_count = 0;
    g_timing.fps_timer = 0.0;

    LOG_DEBUG("Timing system initialized");
}

void timing_frame_start(void) {
    g_timing.frame_start = get_time_seconds();
    g_timing.delta_time = g_timing.frame_start - g_timing.last_frame;

    /* Clamp delta to prevent spiral of death */
    if (g_timing.delta_time > 0.1) {
        g_timing.delta_time = 0.1;
    }
}

void timing_frame_end(int target_fps) {
    double frame_end = get_time_seconds();
    double frame_time = frame_end - g_timing.frame_start;

    /* Limit FPS if requested */
    if (target_fps > 0) {
        double target_frame_time = 1.0 / target_fps;
        if (frame_time < target_frame_time) {
            double sleep_time = target_frame_time - frame_time;
            timing_sleep_ms((int)(sleep_time * 1000.0));
        }
    }

    g_timing.last_frame = get_time_seconds();

    /* Update FPS counter */
    g_timing.frame_count++;
    g_timing.fps_timer += g_timing.delta_time;

    if (g_timing.fps_timer >= 1.0) {
        g_timing.fps = (double)g_timing.frame_count / g_timing.fps_timer;
        g_timing.frame_count = 0;
        g_timing.fps_timer = 0.0;
    }
}

double timing_get_delta(void) {
    return g_timing.delta_time;
}

double timing_get_fps(void) {
    return g_timing.fps;
}

double timing_get_time(void) {
    return get_time_seconds() - g_timing.start_time;
}

void timing_sleep_ms(int ms) {
    if (ms <= 0) return;

    #ifdef _WIN32
        Sleep(ms);
    #else
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    #endif
}
