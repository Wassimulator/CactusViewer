#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <time.h>

// Debug logging system for CactusViewer
// Logs to %APPDATA%\CactusViewer\debug.log

static FILE* g_debug_log_file = nullptr;
static CRITICAL_SECTION g_log_mutex;
static bool g_log_mutex_initialized = false;

static void debug_log_init() {
    if (!g_log_mutex_initialized) {
        InitializeCriticalSection(&g_log_mutex);
        g_log_mutex_initialized = true;
    }

    if (g_debug_log_file == nullptr) {
        char log_path[512];
        char *appdata = getenv("APPDATA");
        if (appdata) {
            snprintf(log_path, sizeof(log_path), "%s\\CactusViewer\\debug.log", appdata);
        } else {
            snprintf(log_path, sizeof(log_path), "debug.log");
        }
        g_debug_log_file = fopen(log_path, "w");
        if (g_debug_log_file) {
            fprintf(g_debug_log_file, "=== CactusViewer Debug Log Started ===\n");
            fflush(g_debug_log_file);
        }
    }
}

static void debug_log_close() {
    if (g_debug_log_file) {
        fprintf(g_debug_log_file, "=== CactusViewer Debug Log Ended ===\n");
        fclose(g_debug_log_file);
        g_debug_log_file = nullptr;
    }
    if (g_log_mutex_initialized) {
        DeleteCriticalSection(&g_log_mutex);
        g_log_mutex_initialized = false;
    }
}

static void debug_log(const char* category, const char* format, ...) {
    if (!g_debug_log_file) {
        debug_log_init();
    }
    if (!g_debug_log_file) return;

    EnterCriticalSection(&g_log_mutex);

    // Get timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);

    // Get thread ID
    DWORD thread_id = GetCurrentThreadId();

    fprintf(g_debug_log_file, "[%02d:%02d:%02d.%03d] [T:%05lu] [%-6s] ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
            thread_id, category);

    va_list args;
    va_start(args, format);
    vfprintf(g_debug_log_file, format, args);
    va_end(args);

    fprintf(g_debug_log_file, "\n");
    fflush(g_debug_log_file);

    LeaveCriticalSection(&g_log_mutex);
}

// Convenience macros for different categories with visual prefixes
#define DLOG_INIT(...)      debug_log("INIT  ", __VA_ARGS__)
#define DLOG_FILE(...)      debug_log("FILE  ", __VA_ARGS__)
#define DLOG_LOADER(...)    debug_log("LOAD  ", __VA_ARGS__)
#define DLOG_THREAD(...)    debug_log("THRD  ", __VA_ARGS__)
#define DLOG_SCAN(...)      debug_log("SCAN  ", __VA_ARGS__)
#define DLOG_SIGNAL(...)    debug_log(">>SIG ", __VA_ARGS__)
#define DLOG_IMAGE(...)     debug_log("IMG   ", __VA_ARGS__)
#define DLOG_WIC(...)       debug_log("WIC   ", __VA_ARGS__)
#define DLOG_WEBP(...)      debug_log("WEBP  ", __VA_ARGS__)
#define DLOG_GIF(...)       debug_log("GIF   ", __VA_ARGS__)
#define DLOG_D3D(...)       debug_log("D3D   ", __VA_ARGS__)
#define DLOG_MUTEX(...)     debug_log("<<MTX ", __VA_ARGS__)
#define DLOG_THUMB(...)     debug_log("THUMB ", __VA_ARGS__)
#define DLOG_SORT(...)      debug_log("SORT  ", __VA_ARGS__)
#define DLOG_ANIM(...)      debug_log("ANIM  ", __VA_ARGS__)
#define DLOG_UI(...)        debug_log("UI    ", __VA_ARGS__)
#define DLOG_ERROR(...)     debug_log("!!ERR ", __VA_ARGS__)
#define DLOG_NAV(...)       debug_log("NAV   ", __VA_ARGS__)

// Helper to log wide strings
static void debug_log_wstr(const char* category, const char* prefix, const wchar_t* wstr) {
    if (!wstr) {
        debug_log(category, "%s: (null)", prefix);
        return;
    }
    char utf8_buf[1024];
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8_buf, sizeof(utf8_buf), NULL, NULL);
    debug_log(category, "%s: %s", prefix, utf8_buf);
}

#define DLOG_WSTR(cat, prefix, wstr) debug_log_wstr(#cat, prefix, wstr)
