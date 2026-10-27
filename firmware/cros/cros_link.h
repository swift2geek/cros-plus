//==========================================================
// cros_link.h
//
// cros-plus: the line-based connection to the host (a
// NAS, a Raspberry Pi, a laptop) that receives EVT lines
// and sends CMD lines. Always the USB serial port; on an
// ESP32 also one TCP client over WiFi.
//==========================================================
//   Licensed under a Creative Commons Attribution-ShareAlike
//   4.0 International License, like the rest of CrOS
//==========================================================
#ifndef CROS_LINK_H
#define CROS_LINK_H

#include <Arduino.h>
#if defined( ESP32 )
#include <WiFi.h>
#endif

//======================================
// TCP port the host connects to (ESP32)
//======================================
#define CROS_LINK_PORT  7777

class CCrowboxLink : public Print
{
public:
    //--------------------------------------
    // Call once from Setup(), after Serial.begin()
    //--------------------------------------
    void Begin();

    //--------------------------------------
    // Call every loop. Returns true when a
    // host has just connected over WiFi, so
    // the core can greet it with its state.
    //--------------------------------------
    bool Poll();

    //--------------------------------------
    // Next received byte, or -1 if none
    //--------------------------------------
    int Read();

    size_t write( uint8_t c ) override;
    size_t write( const uint8_t *pBuffer, size_t size ) override;
    using Print::write;

private:
#if defined( ESP32 )
    WiFiServer m_server;
    WiFiClient m_client;
#endif
};

extern CCrowboxLink g_crOSLink;

#endif//CROS_LINK_H
