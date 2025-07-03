#pragma once

#include "SPI.h"
#include <Arduino_GFX_Library.h>

#define TFT_DC 47
#define TFT_CS 5
#define TFT_SCLK 4
#define TFT_MOSI 2
#define TFT_RST 38
#define TFT_BL 42

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

Arduino_Canvas *backBuffer;

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI);
// Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, -1, SPI_MODE0, 40000000UL); // 40MHz

Arduino_GFX *tft = new Arduino_GC9A01(bus, TFT_RST, 0, true, 240, 240);

extern Audio audio;

void setupDisplay()
{
    Serial.println("Initializing cg display...");
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);
    tft->begin();
    tft->fillScreen(BLACK);
    backBuffer = new Arduino_Canvas(240, 240, tft);
    if (!backBuffer->begin())
    { // Check if allocation successful
        Serial.println("Failed to create back buffer!");
        while (true)
            ; // Halt if no buffer
    }
    backBuffer->fillScreen(BLACK);
}

void showText(const String &status)
{

    tft->fillScreen(BLACK);
    tft->setTextColor(BLUE);
    tft->setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

    int16_t x = (TFT_WIDTH - w) / 2;
    int16_t y = (TFT_HEIGHT - h) / 2;

    tft->setCursor(x, y);
    tft->println(status);
}

void setStatus(const String &status, bool isTop = true)
{
    tft->fillScreen(BLACK);
    tft->setTextColor(WHITE);
    tft->setTextSize(2);

    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(status, 0, 0, &x1, &y1, &w, &h);

    int16_t x = (TFT_WIDTH - w) / 2;
    int16_t y;
    if (isTop)
    {
        y = 90;
    }
    else
    {
        y = TFT_HEIGHT - 100;
    }

    tft->fillRect(0, y, TFT_WIDTH, h, BLACK);
    tft->setCursor(x, y);
    tft->println(status);
}

void drawVUMeter()
{
    uint16_t level = audio.getVUlevel();
    uint16_t minRadius = 0;
    uint16_t maxRadius = TFT_HEIGHT - 100;
    uint16_t radius = map(level, 0, 40000, minRadius, maxRadius);

    backBuffer->fillCircle(TFT_WIDTH / 2, TFT_HEIGHT / 2, radius, GREEN);
}

void scrollText()
{
}

void displayLoop()
{
    backBuffer->fillScreen(BLACK);
    drawVUMeter();
    scrollText();
    backBuffer->flush();
}
