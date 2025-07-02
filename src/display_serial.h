#pragma once

#include <Arduino.h>

void setupDisplay()
{
    Serial.println("Display setup not implemented.");
}

void showText(const String &status)
{
    Serial.println("Display showText not implemented: " + status);
}

void setStatus(const String &status, bool isTop = true)
{
    Serial.println("Display setStatus not implemented: " + status + " isTop: " + (isTop ? "true" : "false"));
}

void displayLoop()
{
   
}