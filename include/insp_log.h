/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   insp_log.h
 * Author: minoc
 *
 * Created on February 3, 2017, 11:43 PM
 */


//! @file insp_log - file containing simple logging framework

#ifndef INSP_LOG_H
#define INSP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

//! Log Message Levels    
#define INSP_LOG_NONE   0  //!< no logging
#define INSP_LOG_FATAL  1  //!< log fatal 
#define INSP_LOG_ERROR  2  //!< log error 
#define INSP_LOG_WARN   3  //!< log warning 
#define INSP_LOG_NOTICE 4  //!< log notice
#define INSP_LOG_DEBUG  5  //!< log debug
#define INSP_LOG__MAX   5  //!< Maximum log level

/*! @def insp_log(log_level, msg, args...) 
    @brief General Log Call Macro - emits stdout msg at specified log level.
  
    @example insp_log(INSP_LOG_ERR, "Invalid Arg, foo=%s", insp_log_null_safe(foo));
*/
#define insp_log(log_level, msg, args...) \
  ((log_level <= gInsp_Log_Level) ? \
  _insp_log_impl(log_level, __FILE__, __LINE__, __FUNCTION__, msg, ##args),0 :\
  0)

//! Set the log level
//! @param log_level - log level
void insp_log_set_level(unsigned int log_level);

//! Get the log level 
//! @return log level
unsigned int insp_log_get_level();

//! set the global flag to log code function, line, file info on logs
//! @param enable - 0 = disable, 1 = enable 
void insp_log_set_code_debug(int enable);

//! get the string form of a given log level
//! @param log_level - log level
//! @return string associated with log_level
const char *insp_log_get_log_level_string(unsigned int log_level);

//! @def INSP_LOG_NULL_SAFE(x)
//! macro useful for showing/logging strings, which may be NULL
#define INSP_LOG_NULL_SAFE(x) \
  (x == NULL ? (const char *)    "(NULL)" : (const char *)    x)

//! @def INSP_LOG_NULL_SAFE_LS(x)
//! macro useful for showing/logging wchar_t *strings, which may be NULL
#define INSP_LOG_NULL_SAFE_LS(x) \
  (x == NULL ? (const wchar_t *) "(NULL)" : (const wchar_t *) x)




// ---------------------------------------------------------------------------
// Internal Log API (exposed only to support the public insp_log macro)
// ---------------------------------------------------------------------------

//! global log level, exposed for insp_log macro
extern unsigned int gInsp_Log_Level;

//! Private, Internal implementation of log macro logic
void _insp_log_impl(
  int           log_level, //!< log level
  const char   *file,      //!< file name
  unsigned int  line,      //!< line number
  const char   *function,  //!< function
  const char   *msg,       //!< msg
                ... )      //!< msg args
#ifdef __GNUC__
          __attribute__ (( format( printf, 5, 6 ) ))
#endif
;

// ---------------------------------------------------------------------------
    
#ifdef __cplusplus
}
#endif
  
#endif /* INSP_LOG_H */
