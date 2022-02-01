/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
#include <string>
#include <sstream>

#ifdef __GNUC__
#include <signal.h>
#define BREAK_HERE raise(SIGINT)
#endif

typedef enum {LOG_VERBOSE = 1, LOG_DEBUG = 2, LOG_INFO = 3, LOG_WARN = 4, LOG_ERROR = 8, LOG_PRINT = 100} logtype_t;

/** log something using standard C printf format varargs - limit 2049 chars */
void doLog(uint32_t t, const char *format, ...);

/** log something using standard C++ std::string - limit 2049 chars */
void doLog(uint32_t t, const std::string &txt);

/** log something using standard C++ std::stringstream - no limit */
void doLog(uint32_t t, const std::stringstream &txt);

/** set the log level ... */
void setLogLevel(uint32_t level);

/** get the log level ... */
uint32_t getLogLevel();
