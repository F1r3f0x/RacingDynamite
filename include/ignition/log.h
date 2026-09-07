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

#ifndef IGNITION_LOG_H
#define IGNITION_LOG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO  = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_NONE  = 4
} LogLevel;

/**
 * Initialize logging subsystem.
 * Opens log file (if filepath provided) and sets initial minimum log level.
 */
bool     Log_Init(const char *filepath, LogLevel min_level);

/**
 * Flush and close active log file and release resources.
 */
void     Log_Shutdown(void);

/**
 * Set minimum active log level.
 */
void     Log_SetLevel(LogLevel level);

/**
 * Query current minimum active log level.
 */
LogLevel Log_GetLevel(void);

/**
 * Enable or disable writing logs to disk file.
 */
void     Log_SetFileLogging(bool enable);

/**
 * Enable or disable writing logs to standard output/error console.
 */
void     Log_SetConsoleLogging(bool enable);

/**
 * Write a formatted log message with level and channel tag.
 */
void     Log_Message(LogLevel level, const char *channel, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 3, 4)))
#endif
;

#define LOG_DEBUG(chan, ...) Log_Message(LOG_LEVEL_DEBUG, chan, __VA_ARGS__)
#define LOG_INFO(chan, ...)  Log_Message(LOG_LEVEL_INFO,  chan, __VA_ARGS__)
#define LOG_WARN(chan, ...)  Log_Message(LOG_LEVEL_WARN,  chan, __VA_ARGS__)
#define LOG_ERROR(chan, ...) Log_Message(LOG_LEVEL_ERROR, chan, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // IGNITION_LOG_H
