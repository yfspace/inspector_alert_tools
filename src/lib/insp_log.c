/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   insp_log.c
 * Author: minoc
 * 
 * Created on February 3, 2017, 11:43 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "insp_log.h"

// ---------------------------------------------------------------------------

// -- Exported Globals --

//! Global Log Level, exported to allow Insp_Log macro to work
unsigned int gInsp_Log_Level = INSP_LOG_WARN;

// -- Module Globals --

//! log messages show file, line, function info in all log messages
int mInsp_Log_Debug_Code = 1; 

//! Inspector Log Levels (must match order from INSP_LOG_* defines.
static const char *mINSP_LOG_LEVELS[] = {
  "LOG_NONE",              
  "FATAL",
  "ERROR", 
  "WARN",
  "NOTICE",
  "DEBUG"
};

// ---------------------------------------------------------------------------

void insp_log_set_level(unsigned int log_level) 
{
  if (log_level > INSP_LOG__MAX) 
    log_level = INSP_LOG__MAX;

  gInsp_Log_Level = log_level;
}

// ---------------------------------------------------------------------------

unsigned int insp_log_get_level() { 
  return gInsp_Log_Level;
}

// ---------------------------------------------------------------------------

void insp_log_set_code_debug(int enable)
{
  mInsp_Log_Debug_Code = (enable ? 1 : 0);
}

// ---------------------------------------------------------------------------

const char *insp_log_get_log_level_string(unsigned int log_level)
{
  if (log_level > INSP_LOG__MAX) 
    log_level = INSP_LOG__MAX;

  return mINSP_LOG_LEVELS[log_level];
}

// ---------------------------------------------------------------------------

void _insp_log_impl(
  int log_level,
  const char *file,
  unsigned int line,
  const char *function,
  const char *msg,
              ...)
{
  if (log_level > INSP_LOG__MAX) 
    log_level = INSP_LOG__MAX;

  if (mInsp_Log_Debug_Code) {
    fprintf(stderr, "[%s:%u (%s)] %s - ", 
            file, line, function, 
            mINSP_LOG_LEVELS[log_level]);
  } else {
    fprintf(stderr, "%s - ", 
            mINSP_LOG_LEVELS[log_level]);
  }

  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");

  if (log_level == INSP_LOG_FATAL) {
    fprintf(stderr, "ABORTING\n");
    fflush(stderr);
    exit(-1);
  }
}

// ---------------------------------------------------------------------------

