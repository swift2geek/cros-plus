//==========================================================
// cros_constants.h 
//
// Fixed values used throughout
//
// Version 1.0 - May 2025
//
// http://www.thecrowbox.com
//==========================================================
//   Except where otherwise noted, this work is licensed 
//   under a Creative Commons Attribution-ShareAlike 4.0 
//   International License
//==========================================================
#ifndef CROS_CONSTANTS_H
#define CROS_CONSTANTS_H

#include "cros_core.h"

//======================================
// Constants used by the code that stores
// the currently selected phase of training
// in the UNO's EEPROM so that it will be
// remembered when the CrowBox is turned
// off
//======================================
#define CROS_EEPROM_HEADER_STRING           "CrOS"
#define CROS_EEPROM_ADDRESS_HEADER          0
#define CROS_EEPROM_ADDRESS_TRAINING_PHASE  4

//======================================
// Default baud rate used for serial
// communications. The main application 
// for serial communication in this early
// release is just reporting text-based
// error and update messages to the console
//======================================
#define CROS_SERIAL_BAUD_RATE  57600

//======================================
// This value should always be #UNDEF 
// when compiling for a CrowBox that is 
// being deployed. If you'd like debug
// messages to appear in the serial 
// monitor, then #define this while you
// debug. Just remember to #undef after.
//======================================
#undef CROS_USE_SERIAL_DEBUG
//#define CROS_USE_SERIAL_DEBUG

//======================================
// cros-plus: one-line machine-readable events
// for the host (USB serial, or WiFi on ESP32):
//   EVT LAND        a bird landed on the perch
//   EVT LEAVE       the perch has been empty for a moment
//   EVT COIN        a coin deposit was detected
//   EVT FEED        the reward basket opened
//   EVT CLOSE       the reward basket finished closing
//   EVT PHASE <n>   current training phase (on boot and on change)
//   EVT ALIVE <s>   heartbeat with uptime in seconds
// Hosts ignore every line that does not
// start with "EVT ", so debug output may
// be enabled at the same time.
//======================================
#define CROS_USE_SERIAL_EVENTS

//======================================
// cros-plus: the perch must stay released this
// long before the next press counts as a new
// arrival, so a bird hopping around on the
// perch is reported only once.
//======================================
#define EVENT_PERCH_DEPARTURE_DELAY  2.0f

//======================================
// cros-plus: commands from the host, one
// per line on the same serial port:
//   CMD STATUS       reply: EVT STATUS phase=<n> basket=<open|closed> perch=<0|1> queued=<n> uptime=<s>
//   CMD PHASE <n>    switch to training phase 1-4 (same as the button)
//   CMD OPEN         open the lid; it closes again by itself as usual
//   CMD CLOSE        close the lid now (refused in phase 1, which keeps it open)
// A command that can't be carried out is answered
// with EVT ERR <command>.
//======================================
#define CROS_USE_SERIAL_COMMANDS
#define CROS_COMMAND_MAX_LENGTH  24

//======================================
// cros-plus: how often EVT ALIVE is sent, so
// the host notices a dead or unplugged box
//======================================
#define EVENT_HEARTBEAT_INTERVAL  60.0f

//======================================
// cros-plus: hardware watchdog. If Loop()
// stops running for 8 seconds (a hang,
// a glitch from a noisy servo supply),
// the Arduino reboots on its own and
// parks the lid again instead of sitting
// frozen until someone walks out to it.
//======================================
#define CROS_USE_WATCHDOG

//======================================
// How long an 'ideal' loop should take
// to execute. If a Loop() call takes 
// less time than this, we'll delay for
// the balance. This forces a 'frame rate'
// for the system and its main function is
// to avoid problems with contact bounce
// when reading physical switches.
//
// This feature is scheduled for removal
// in the near future now that the critical 
// switches (the perch and the coin sensor) 
// have been set up as interrupts.
//======================================
#define CROS_IDEAL_LOOP_MS  20  // 20ms, Roughly 50'fps'

//======================================
// General behavior parameters
//======================================
#define TIME_NEVER    -1.0f

//======================================
// State of the lid which controls access
// to the basket containing rewards
//======================================
#define BASKET_STATE_DONT_KNOW  -1
#define BASKET_STATE_OPEN       1
#define BASKET_STATE_CLOSED     0

#define BASKET_REMAIN_OPEN_DURATION      15.0f

//======================================
// Video Record
//======================================
#define VIDEO_RECORD_DURATION_ARRIVAL   15
#define VIDEO_RECORD_DURATION_DEPOSIT   10

//======================================
// Training Phases
//======================================
#define PHASE_ONE   1
#define PHASE_TWO   2
#define PHASE_THREE 3
#define PHASE_FOUR  4

//======================================
// Pin apportioning
//======================================
// The following assumes Arduino UNO. These 
// will have to be changed if porting this 
// code to run on another Arduino board, 
// or on one of the Particle boards. 
// NOTE: The input pins assigned to COIN
// must support interrupts!
#if defined( ESP32 )
// cros-plus: ESP32 DevKit (ESP32-WROOM-32). Every GPIO
// below has an internal pull-up and interrupts, and none
// of them is a boot strapping pin except the LED on GPIO2,
// which is the on-board LED of most DevKit clones.
#define OUTPUT_PIN_LED          2
#define OUTPUT_PIN_SERVO        18
#define INPUT_PIN_PERCH         25
#define INPUT_PIN_COIN          26
#define INPUT_PIN_PHASE_SELECT  27
#else
#define OUTPUT_PIN_LED          13
#define OUTPUT_PIN_SERVO        9
#define INPUT_PIN_PERCH         2
#define INPUT_PIN_COIN          3
#define INPUT_PIN_PHASE_SELECT  4
#endif

//======================================
// cros-plus: one coin deposit per this many
// milliseconds at most (contact bounce on
// the copper coin rail). Integer, because
// the ESP32 must not touch floats inside
// an interrupt.
//======================================
#define COIN_DEBOUNCE_MS  1000UL

//======================================
// cros-plus: interrupt code has to live in
// IRAM on an ESP32
//======================================
#if defined( ESP32 )
#define CROS_ISR_ATTR  IRAM_ATTR
#else
#define CROS_ISR_ATTR
#endif

//======================================
// Constants for servo behavior
//======================================
#define SERVO_POS_OPEN        180
#define SERVO_POS_CLOSED      0     
#define SERVO_POS_MIDPOINT    90

//======================================
// Reward Basket behavior
//
// Think very carefully before changing
// either of these values. These values 
// define a safety feature which directs
// the sliding lid to close slowly in a
// series of small, staggered steps. The
// purpose of this behavior is to give 
// birds plenty of notice that the lid
// is closing before there's any danger
// of a bird getting one of their parts
// caught in the closing lid.
//======================================
#define BASKET_CLOSE_NUM_STEPS       15  // How many steps it takes to close the basket [NOTE: ensure SERVO_POS_OPEN % BASKET_CLOSE_NUM_STEPS == 0]
#define BASKET_CLOSE_STEP_DELAY_MS   750 // How long (ms) to wait between each 'step' when closing

//======================================
// Enumerated values for various error 
// types. These are used to determine how
// many times to blink the LED when we're
// stuck in the SystemError() function
//======================================
typedef enum
{
  kError_Generic = 2,
  kError_SetupFailed,  
  kError_BadTrainingPhase,
  kError_EEPROM,
} cros_error_code_t;


#endif//CROS_CONSTANTS_H

