/*
 * Racing Dynamite - Modern open-source source port of Ignition (1997)
 * Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ignition/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

static FILE     *s_log_file = NULL;
static LogLevel  s_min_level = LOG_LEVEL_DEBUG;
static bool      s_log_to_file = true;
static bool      s_log_to_console = true;
static uint64_t  s_start_time_ms = 0;

static uint64_t GetCurrentTimeMs(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

static const char *LevelToString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "UNKN ";
    }
}

bool Log_Init(const char *filepath, LogLevel min_level) {
    s_min_level = min_level;
    s_start_time_ms = GetCurrentTimeMs();

    if (s_log_file) {
        fclose(s_log_file);
        s_log_file = NULL;
    }

    if (filepath && filepath[0] != '\0') {
        s_log_file = fopen(filepath, "w");
        if (!s_log_file) {
            fprintf(stderr, "[Log] Failed to open log file: %s\n", filepath);
            s_log_to_file = false;
            return false;
        }
        s_log_to_file = true;

        // Write header
        time_t raw_time = time(NULL);
        struct tm *time_info = localtime(&raw_time);
        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", time_info);

        fprintf(s_log_file, "================================================================================\n");
        fprintf(s_log_file, " Racing Dynamite (Ignition 1997 Source Port) Diagnostic Session Log\n");
        fprintf(s_log_file, " Session Started: %s\n", time_buf);
        fprintf(s_log_file, "================================================================================\n\n");
        fflush(s_log_file);
    } else {
        s_log_to_file = false;
    }

    return true;
}

void Log_Shutdown(void) {
    if (s_log_file) {
        uint64_t elapsed = GetCurrentTimeMs() - s_start_time_ms;
        uint32_t sec = (uint32_t)(elapsed / 1000);
        uint32_t ms = (uint32_t)(elapsed % 1000);

        fprintf(s_log_file, "\n================================================================================\n");
        fprintf(s_log_file, " Session Ended. Total Run Time: %u.%03u seconds\n", sec, ms);
        fprintf(s_log_file, "================================================================================\n");
        fflush(s_log_file);
        fclose(s_log_file);
        s_log_file = NULL;
    }
}

void Log_SetLevel(LogLevel level) {
    s_min_level = level;
}

LogLevel Log_GetLevel(void) {
    return s_min_level;
}

void Log_SetFileLogging(bool enable) {
    s_log_to_file = enable;
}

void Log_SetConsoleLogging(bool enable) {
    s_log_to_console = enable;
}

void Log_Message(LogLevel level, const char *channel, const char *fmt, ...) {
    if (level < s_min_level || level == LOG_LEVEL_NONE) {
        return;
    }

    uint64_t elapsed = GetCurrentTimeMs() - s_start_time_ms;
    uint32_t total_sec = (uint32_t)(elapsed / 1000);
    uint32_t minutes = total_sec / 60;
    uint32_t seconds = total_sec % 60;
    uint32_t ms = (uint32_t)(elapsed % 1000);

    const char *lvl_str = LevelToString(level);
    const char *chan_str = (channel && channel[0] != '\0') ? channel : "SYSTEM";

    char msg_buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    // Strip trailing newline if present to keep formatting uniform
    size_t len = strlen(msg_buf);
    while (len > 0 && (msg_buf[len - 1] == '\n' || msg_buf[len - 1] == '\r')) {
        msg_buf[--len] = '\0';
    }

    // Console output
    if (s_log_to_console) {
        FILE *target = (level >= LOG_LEVEL_ERROR) ? stderr : stdout;
        fprintf(target, "[%02u:%02u.%03u] [%s] [%s] %s\n",
                minutes, seconds, ms, lvl_str, chan_str, msg_buf);
        fflush(target);
    }

    // Disk file output
    if (s_log_to_file && s_log_file) {
        fprintf(s_log_file, "[%02u:%02u.%03u] [%s] [%s] %s\n",
                minutes, seconds, ms, lvl_str, chan_str, msg_buf);
        fflush(s_log_file); // Critical: guarantee logs are immediately on disk
    }
}
