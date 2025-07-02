#pragma once

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"

#define TFT_DC 47
#define TFT_CS 5
#define TFT_SCLK 4
#define TFT_MOSI 2
#define TFT_RESET 38
#define TFT_BL 42

Adafruit_GC9A01A tft(TFT_CS, TFT_DC);

void setupDisplay()
{

    Serial.println("Initializing cg display...");
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);
    tft.begin();
    tft.fillScreen(GC9A01A_BLACK);
}

void showText(const String &status)
{

    tft.fillScreen(GC9A01A_BLACK);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(2);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

    int16_t x = (tft.width() - w) / 2;
    int16_t y = (tft.height() - h) / 2;

    tft.setCursor(x, y);
    tft.println(status);
}

void setStatus(const String &status, bool isTop = true)
{
    tft.fillScreen(GC9A01A_BLACK);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(2);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

    int16_t x = (tft.width() - w) / 2;
    int16_t y;
    if (isTop)
    {
        y = 0;
    }
    else
    {
        y = tft.height() - h;
    }

    tft.fillRect(0, y, tft.width(), h, GC9A01A_BLACK); // Clear the area
    tft.setCursor(x, y);
    tft.println(status);
}

void displayLoop()
{
}