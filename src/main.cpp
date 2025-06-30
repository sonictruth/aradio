#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "Audio.h"
#include "webroutes.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define I2S_DOUT 47
#define I2S_BCLK 21
#define I2S_LRC 38

#define I2C_SDA 17
#define I2C_SCL 18

#define BLUE_LED_PIN 2

#define BUTTON_PIN 0

String deviceName = "ARadio";
String devicePassword = "12345678";

char topStatus[256] = "-";
char bottomStatus[256] = "-";

char stationName[256] = "-";
char stationTitle[256] = "-";

String webURL = "";

int topStatusTextWidth = 0;
int bottomStatusTextWidth = 0;
int topStatusTextX = 0;
int bottomStatusTextX = 0;
int topScrollSpeed = 20;
int bottomScrollSpeed = 20;

Audio audio;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool isMDNSEnabled = true;

void setupDisplay()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
  }
  display.setTextColor(SSD1306_WHITE);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0x01);
}

void showText(const String &status)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextWrap(true);
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (SCREEN_WIDTH - w) / 2;
  int16_t y = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(x, y);
  display.println(status);
  display.display();
}

void setStatus(const String &status, bool isTop = true)
{
  display.setTextWrap(false);
  char *targetStatus = isTop ? topStatus : bottomStatus;
  size_t bufSize = isTop ? sizeof(topStatus) : sizeof(bottomStatus);

  strncpy(targetStatus, status.c_str(), bufSize - 1);
  targetStatus[bufSize - 1] = '\0';

  if (status.length() >= bufSize)
  {
    targetStatus[bufSize - 1] = '\0';
  }

  int16_t x1, y1;
  uint16_t w, h;
  if (isTop)
  {
    display.setTextSize(1);
  }
  else
  {
    display.setTextSize(2);
  }
  display.getTextBounds(targetStatus, 0, 0, &x1, &y1, &w, &h);
  if (isTop)
  {
    topStatusTextWidth = (int)w;
    topStatusTextX = SCREEN_WIDTH;
  }
  else
  {
    bottomStatusTextWidth = (int)w;
    bottomStatusTextX = SCREEN_WIDTH;
  }
}

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

  if (!MDNS.begin(deviceName))
  {
    Serial.println("Error setting up MDNS responder!");
    isMDNSEnabled = false;
  }
  else
  {
    Serial.println("mDNS responder started");
    isMDNSEnabled = true;
  }
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("Starting...");
  Serial.print("Total PSRAM: ");
  Serial.println(ESP.getPsramSize());

  setupDisplay();

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);
  setupWifi();

  String hostName = WiFi.localIP().toString();

  setupWebServer();
  if (isMDNSEnabled)
  {
    hostName = deviceName + ".local";
  }

  webURL = "http://" + hostName;

  audio.connecttospeech("Hello, this is a test speech", "en");

 // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3");
}

void scrollText()
{

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(topStatusTextX, 0);
  display.print(topStatus);

  display.setTextSize(2);
  display.setCursor(bottomStatusTextX, 17);
  display.print(bottomStatus);

  display.display();

  topStatusTextX -= topScrollSpeed;
  bottomStatusTextX -= bottomScrollSpeed;

  if (topStatusTextX < -topStatusTextWidth)
  {
    topStatusTextX = SCREEN_WIDTH;
  }

  if (bottomStatusTextX < -bottomStatusTextWidth)
  {
    bottomStatusTextX = SCREEN_WIDTH;
  }
}

void loop()
{
  vTaskDelay(1);
  audio.loop();
  
  static unsigned long lastScroll = 0;
  if (millis() - lastScroll >= 500)
  {
    scrollText();
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
