//==========================================================
// cros_core.h
//
// The 'kernel' of the CrOS is called "The Core"
//
// Version 1.0 - May 2025
//
// http://www.thecrowbox.com
//==========================================================
//   Except where otherwise noted, this work is licensed 
//   under a Creative Commons Attribution-ShareAlike 4.0 
//   International License
//==========================================================
#ifndef CROS_CORE_H
#define CROS_CORE_H

#include <Arduino.h>
#include <Servo.h>
#include "cros_types.h"
#include "cros_constants.h"

//==========================================================
// This fancy 'core' of ours is actually
// just an instance of this CCrowboxCore
// object.
//==========================================================
class CCrowboxCore
{
public:
    //--------------------
    // CTor/DTor
    //--------------------
    CCrowboxCore();
    
    //--------------------------------------
    // To promote understanding, these methods
    // bear the same name as the Arduino
    // entry points which call them.
    //--------------------------------------
    void Setup();
    void Loop();

public:
    void DebugPrint( const char *pString );
    void ReportEvent( const char *pName );
    void ReportEvent( const char *pName, int value );
    void ReportEvent( const char *pName, unsigned long value );
    void ReportSystemError( cros_error_code_t errorCode );
    
    //--------------------------------------
    // Public Accessor methods
    //--------------------------------------
    cros_time_t GetUptimeSeconds();
    bool        IsABirdOnThePerch();
    cros_time_t HowLongHasBirdBeenHere();
    cros_time_t HowLongHasBirdBeenGone();
    
    bool  IsRewardBasketOpen() const    { return m_basketState == BASKET_STATE_OPEN; }
    
    unsigned char   GetCurrentTrainingPhase() const { return m_currentTrainingPhase; }    
    void            ReportCurrentTrainingPhase();
    
    //--------------------------------------
    // Coins & Rewards
    //--------------------------------------
    bool EnqueueCoin();
    void RemoveEnqueuedCoin();
    
protected:
    //--------------------------------------
    // Native Methods
    //--------------------------------------
    void BlinkLED( int numTimes );
    void AttachBasketServo();
    void DetachBasketServo();
    
    void OpenRewardBasket();
    void CloseRewardBasket();
    
    bool Poll_IsPerchPressed();
    
    void ScheduleBasketCloseWithDelay( cros_time_t delayInSeconds );

    static bool HasElapsed( cros_ms_t since, cros_time_t seconds );

    //--------------------------------------
    // Training Protocol
    //--------------------------------------
    void RunPhaseOneProtocol();
    void RunPhaseTwoProtocol();
    void RunPhaseThreeProtocol();
    void RunPhaseFourProtocol();
    void AdvanceCurrentTrainingPhase();
    void ApplyTrainingPhaseChange();

    void CheckTrainingPhaseSwitch();

    //--------------------------------------
    // CrOS-Pi: serial events for the Pi
    //--------------------------------------
    void ReportPerchEvents();
    void ReportCoinEvents();
    void ReportHeartbeat();

    //--------------------------------------
    // CrOS-Pi: serial commands from the Pi
    //--------------------------------------
    void ProcessSerialCommands();
    void ExecuteCommand( const char *pCommand );
    void ReportStatus();

    //--------------------------------------
    // EEPROM Methods
    //--------------------------------------
    bool  ValidateEEPROMData();
    void  CreateEEPROMData();
    void  LoadCurrentTrainingPhaseFromEEPROM();
    void  WriteCurrentTrainingPhaseToEEPROM();

    //--------------------------------------
    // Video
    //--------------------------------------
    void RecordVideo( cros_time_t duration );
    void StopRecordingVideo();

private:
    //--------------------------------------
    // CrOS-Pi: all moments are millis() values
    // paired with a flag, instead of float
    // uptimes compared against each other,
    // which broke after ~49.7 days of running
    //--------------------------------------
    cros_ms_t   m_msBirdLanded;
    cros_ms_t   m_msBirdDeparted;
    bool        m_isBirdOnPerch;
    bool        m_hasBirdDeparted;

    cros_ms_t   m_msBasketCloseScheduled;
    cros_time_t m_basketCloseDelay;
    bool        m_isBasketCloseScheduled;

    volatile cros_ms_t m_msLastCoinDetected;

    //--------------------------------------
    // Uptime that survives millis() wrapping
    //--------------------------------------
    cros_ms_t    m_msLastLoopBegan;
    unsigned int m_numMillisWraps;

    //--------------------------------------
    // This semaphore is used to begin recording video
    // or to extend recording time if already doing so
    //--------------------------------------
    cros_ms_t   m_msVideoStarted;
    cros_time_t m_videoDuration;
    bool        m_isRecordingVideo;

    //--------------------------------------
    // Indicates which (1,2,3 or 4) training
    // phase the box is currently observing
    //--------------------------------------
    unsigned char m_currentTrainingPhase;

    //--------------------------------------
    // Used to track the current state of 
    // the basket. Whether it's open or 
    // closed, for instance.
    //--------------------------------------
    int m_basketState;
    
    //--------------------------------------
    // Each time a coin hits the coin deposit
    // sensor, we increment this value. We
    // decrement this each time we open the 
    // reward basket (Ph3+) so if several coins
    // are dropped at about the same time,
    // this will rack up three full cycles of
    // the door in order to square up the debt
    //--------------------------------------
    volatile int m_numEnqueuedDeposits;

    //--------------------------------------
    // CrOS-Pi: coin deposits counted by the
    // interrupt but not yet reported over
    // serial (the interrupt can't touch the
    // serial port), and the perch state used
    // to report arrivals in every phase.
    //--------------------------------------
    volatile int m_numUnreportedDeposits;
    bool         m_isPerchOccupied;
    cros_ms_t    m_msPerchLastPressed;
    cros_ms_t    m_msLastHeartbeat;

    //--------------------------------------
    // CrOS-Pi: the command line being received.
    // A line longer than the buffer is dropped
    // as a whole.
    //--------------------------------------
    char          m_commandBuffer[ CROS_COMMAND_MAX_LENGTH ];
    unsigned char m_commandLength;
    bool          m_isCommandOverflowing;

    //--------------------------------------
    // The servo which controls the lid that
    // covers the reward basket.
    //--------------------------------------
    Servo m_basketServo;
};

extern CCrowboxCore g_crOSCore;

#endif//CROS_CORE_H

