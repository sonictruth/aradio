#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define I2C_SDA 17
#define I2C_SCL 18
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define I2S_DOUT 47
#define I2S_BCLK 21
#define I2S_LRC 38

int topStatusTextWidth = 0;
int bottomStatusTextWidth = 0;
int topStatusTextX = 0;
int bottomStatusTextX = 0;
int topScrollSpeed = 20;
int bottomScrollSpeed = 20;

extern char topStatus[256] = "";
extern char bottomStatus[256] = "";

extern char stationName[256] = "";
extern char stationTitle[256] = "";
extern char lastStreamURL[256] = "";

extern char localWebUIURL[200] = "";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setupDisplay()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    delay(5000);
    ESP.restart();
  }
  display.setTextColor(SSD1306_WHITE);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0x01);
  topStatusTextX = SCREEN_WIDTH;
  bottomStatusTextX = SCREEN_WIDTH;
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
  char *targetStatus = isTop ? topStatus : bottomStatus;
  size_t bufSize = isTop ? sizeof(topStatus) : sizeof(bottomStatus);

  if (isTop) {
    snprintf(targetStatus, bufSize, "%s Connect @ %s", status, localWebUIURL);
  } else {

    strncpy(targetStatus, status.c_str(), bufSize - 1);
    targetStatus[bufSize - 1] = '\0'; 
  }



  display.setTextWrap(false);

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

void displayLoop()
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
