//==========================================================
// cros_types.h
//
// Home to common datatypes used throughout the system
//
// Version 1.0 - May 2025
//
// http://www.thecrowbox.com
//==========================================================
//   Except where otherwise noted, this work is licensed 
//   under a Creative Commons Attribution-ShareAlike 4.0 
//   International License
//==========================================================
#ifndef CROS_TYPES_H
#define CROS_TYPES_H

//--------------------------------------
// Time stored in a float, formatted as 
// seconds.fractions of second
//
// This is generally used throughtout 
// CrOS code as it's a more convenient 
// format than dealing with millisecond
// time
//--------------------------------------
typedef float cros_time_t;

//--------------------------------------
// cros-plus: a moment in time, as returned
// by millis(). Two of these are only ever
// compared by subtracting them (see
// HasElapsed()), which stays correct when
// millis() wraps around after ~49.7 days.
//--------------------------------------
typedef unsigned long cros_ms_t;

#endif//CROS_TYPES_H

