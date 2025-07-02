#include <Arduino.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include "webroutes.h"
#include "epromAddreses.h"

#include "display_serial.h"
#include "audio_es8311.h"

// #include "display_sdd1306.h"
// #include "audio_pcm5102.h"


#define ENABLE_MDNS 0

#define EEPROM_SIZE 512


String deviceName = "ARadio";
String devicePassword = "12345678";

char topStatus[256] = "";
char bottomStatus[256] = "";

char stationName[256] = "";
char stationTitle[256] = "";
char lastStreamURL[256] = "";

char localWebUIURL[200] = "";



void setupWifi()
{
  showText("Connecting to WiFi...");
  WiFiManager wm;
  bool res;

  wm.setAPCallback([](WiFiManager *myWiFiManager)
                   { showText("Configure Wifi. Connect to: " + String(deviceName) + " Pass: " + String(devicePassword)); });

  res = wm.autoConnect(deviceName.c_str(), devicePassword.c_str());

  if (!res)
  {
    Serial.println("Failed to connect");
    showText("Cannot connect to WiFi. Restarting...");
    delay(5000);
    ESP.restart();
  }
  else
  {
    showText("Connected to Wifi");
  }

  String hostName = WiFi.localIP().toString();

#if ENABLE_MDNS
  if (MDNS.begin(deviceName.c_str()))
  {
    hostName = deviceName + ".local";
  }
#endif

  snprintf(localWebUIURL, sizeof(localWebUIURL), "http://%s", hostName.c_str());
}

void setup()
{

  Serial.begin(115200);
  Serial.println("Starting...");
  Serial.print("Total PSRAM: ");
  Serial.println(ESP.getPsramSize());

  setupDisplay();
  setupAudio();
  setupWifi();

  EEPROM.begin(EEPROM_SIZE);

  int volume = EEPROM.readInt(VOLUME_EPROM_ADDRESS);
  Serial.print("Volume from EEPROM: ");
  Serial.println(volume);
  if (volume > 0 && volume <= 21)
  {
    audio.setVolume(volume);
  }
  else
  {
    audio.setVolume(12);
  }

  setupWebServer();

  EEPROM.readString(0, lastStreamURL, sizeof(lastStreamURL));

  if (strlen(lastStreamURL) > 0)
  {
    audio.connecttohost(lastStreamURL);
    setStatus("Resuming: " + String(lastStreamURL), true);
  }
  else
  {
    setStatus("No previous stream found", true);
    setStatus(localWebUIURL, false);
  }
}

void loop()
{
  vTaskDelay(1);
  audio.loop();

  static unsigned long lastScroll = 0;
  if (millis() - lastScroll >= 500)
  {
    displayLoop();
    lastScroll = millis();
  }
}

void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info)
{
  Serial.print("id3data     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info)
{
  Serial.print("eof_mp3     ");
  Serial.println(info);
}
void audio_showstation(const char *info)
{
  Serial.print("station     ");
  Serial.println(info);
  strncpy(stationName, info, sizeof(stationName) - 1);
  stationName[sizeof(stationName) - 1] = '\0';
  setStatus(stationName, true);
}
void audio_showstreamtitle(const char *info)
{
  Serial.print("streamtitle ");
  Serial.println(info);
  strncpy(stationTitle, info, sizeof(stationTitle) - 1);
  stationTitle[sizeof(stationTitle) - 1] = '\0';
  setStatus(stationTitle, false);
}
void audio_bitrate(const char *info)
{
  Serial.print("bitrate     ");
  Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}
