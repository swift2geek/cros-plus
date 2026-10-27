//==========================================================
// cros_link.cpp
//
// cros-plus: see cros_link.h
//==========================================================
//   Licensed under a Creative Commons Attribution-ShareAlike
//   4.0 International License, like the rest of CrOS
//==========================================================
#include "cros_link.h"
#if defined( ESP32 )
#if !__has_include( "wifi_config.h" )
#error "ESP32 build: copy wifi_config.example.h to wifi_config.h and fill in your WiFi"
#endif
#include "wifi_config.h"
#endif

CCrowboxLink g_crOSLink;

//----------------------------------------------------------
// Start joining the WiFi and listening for the host. Never
// blocks: the box must feed birds even without a network,
// and the ESP32 keeps reconnecting in the background.
//----------------------------------------------------------
void CCrowboxLink::Begin()
{
#if defined( ESP32 )
  WiFi.mode( WIFI_STA );
  WiFi.setHostname( CROS_WIFI_HOSTNAME );
  WiFi.setAutoReconnect( true );
  WiFi.begin( CROS_WIFI_SSID, CROS_WIFI_PASSWORD );

  m_server.begin( CROS_LINK_PORT );
  m_server.setNoDelay( true );
#endif
}

//----------------------------------------------------------
// One host at a time: a new connection replaces the old one,
// so a host that crashed without closing its socket can
// simply reconnect.
//----------------------------------------------------------
bool CCrowboxLink::Poll()
{
#if defined( ESP32 )
  WiFiClient client = m_server.accept();
  if( client )
  {
    m_client.stop();
    m_client = client;
    m_client.setNoDelay( true );
    return true;
  }
#endif
  return false;
}

//----------------------------------------------------------
//----------------------------------------------------------
int CCrowboxLink::Read()
{
  if( Serial.available() > 0 )
  {
    return Serial.read();
  }
#if defined( ESP32 )
  if( m_client.connected() && m_client.available() > 0 )
  {
    return m_client.read();
  }
#endif
  return -1;
}

//----------------------------------------------------------
//----------------------------------------------------------
size_t CCrowboxLink::write( uint8_t c )
{
  return write( &c, 1 );
}

//----------------------------------------------------------
// Everything goes to the USB serial port, and to the WiFi
// host when one is connected.
//----------------------------------------------------------
size_t CCrowboxLink::write( const uint8_t *pBuffer, size_t size )
{
  Serial.write( pBuffer, size );
#if defined( ESP32 )
  if( m_client.connected() )
  {
    m_client.write( pBuffer, size );
  }
#endif
  return size;
}
