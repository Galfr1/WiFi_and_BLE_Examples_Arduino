#include <ArduinoBLE.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "utility/wifi_drv.h"

//Change here to your wifi network credentials:
char ssidName[] = ""; //wifi network name (case sensitive)
char ssidPass[] = ""; //wifi network password (case sensitive)

WiFiServer server( 80 );
int status = WL_IDLE_STATUS;
bool networkInitialized = true;
bool wifiModeFlag = true;

const int LED_PIN = LED_BUILTIN;

bool buttonState = false;

void setup()
{
  Serial.begin( 9600 );
  while ( !Serial );

  pinMode( LED_PIN, OUTPUT );
}

void loop()
{
  if( !networkInitialized )
  {
    if( !wifiModeFlag )
    {
      Serial.print( "Switch to BLE: " );
      if( !switch2BleMode() )
      {
        Serial.println( "failed" );
      }
      else
      {
        networkInitialized = true;
        Serial.println( "success" );
      }
    }
    else
    {
      Serial.print( "Switch to WiFi: " );
      if( !switch2WiFiMode() )
      {
        Serial.println( "failed" );
      }
      else
      {
        networkInitialized = true;
        Serial.println( "success" );
      }
    }
  }
  else
  {
    if( !wifiModeFlag )
    {
      bleMode();
    }
    else
    {
      wifiMode();
    }
  }
}

void controlLed(BLEDevice peripheral) {

  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    doomReset();
  }

  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    doomReset();
  }

  BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    doomReset();
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    doomReset();
  }

  if (peripheral.connected()) {

      if (buttonState) {
        Serial.println("LED ON");

        ledCharacteristic.writeValue((byte)0x01);
      } else {
        Serial.println("LED OFF");

        ledCharacteristic.writeValue((byte)0x00);
      }
    }
  

  Serial.println("Peripheral disconnected");
  doomReset();
}

void doomReset(){
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);
    digitalWrite(12, LOW);
}

void bleMode()
{
  BLEDevice peripheral = BLE.available();

  if (peripheral) {

    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "LED") {
      doomReset();
    }

    BLE.stopScan();

    controlLed(peripheral);
  }
}

void wifiMode()
{
  int connectCount = 0;

  if ( status != WL_CONNECTED )
  {
    while ( status != WL_CONNECTED )
    {
      connectCount++;
      Serial.print( "WiFi attempt: " );
      Serial.println( connectCount );

      if( connectCount > 10 )
      {
        networkInitialized = false;
        wifiModeFlag = false;
        Serial.println( "WiFi connection failed" );
        doomReset();
      }
      Serial.print( "Attempting to connect to SSID: " );
      Serial.println( ssidName );

      status = WiFi.begin( ssidName, ssidPass );

      if( status != WL_CONNECTED )
      {
        delay( 10000 );
      }
    }
    printWiFiStatus();

    server.begin();
  }
  else
  {
    WiFiClient client = server.available();

    if ( client )
    {
      String currentLine = "";
      while ( client.connected() )
      {
        if ( client.available() )
        {
          char c = client.read();
          if ( c == '\n' )
          {
            if ( currentLine.length() == 0 )
            {
              client.println( "HTTP/1.1 200 OK" );
              client.println( "Content-type:text/html" );
              client.println();

              client.print( "<a href=\"/H\">ON  </a>" );
              client.print( "<a href=\"/L\">OFF </a>" );
              client.print( "<a href=\"/B\">SEND  </a>" );

              client.println();
              break;
            }
            else 
            {
              if ( currentLine.startsWith( "GET /H" ) )
              {
                buttonState = true;
              }
              if ( currentLine.startsWith( "GET /L" ) )
              {
                buttonState = false;
              }
              if ( currentLine.startsWith( "GET /B" ) )
              {
                networkInitialized = false;
                wifiModeFlag = false;
              }
              currentLine = "";
            }
          }
          else if ( c != '\r' )
          {
            currentLine += c;
          }
        }
      }
      client.stop();
    }
  }
}

bool switch2BleMode()
{
  if ( !BLE.begin() )
  {
    return false;
  }

  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");

  return true;
}


bool switch2WiFiMode()
{
  BLE.end();

  status = WL_IDLE_STATUS;

  wiFiDrv.wifiDriverDeinit();
  wiFiDrv.wifiDriverInit();

  return true;
}

void printWiFiStatus()
{
  Serial.print( "SSID: " );
  Serial.println( WiFi.SSID() );

  IPAddress ip = WiFi.localIP();
  Serial.print( "IP address: " );
  Serial.println( ip );

  long rssi = WiFi.RSSI();
  Serial.print( "Signal strength (RSSI):" );
  Serial.print( rssi );
  Serial.println( " dBm" );
}