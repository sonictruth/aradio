#pragma once
#include <ESPAsyncWebServer.h>
#include "Audio.h"

AsyncWebServer server(80);
extern Audio audio;

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

inline void setupWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "<html><body></body></html>"); });

    server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "OK"); });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              if (!audio.isRunning())
              {
                request->send(400, "text/plain", "Audio not running");
                return;
              }
              else
              {
                audio.stopSong();
                request->send(200, "text/plain", "Stopped");
              } });

    server.on("/setvolume", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              if (request->hasParam("value")) {
                String valueStr = request->getParam("value")->value();
                int vol = valueStr.toInt();
                if (vol >= 0 && vol <= 21) {
                  audio.setVolume(vol);
                  request->send(200, "text/plain", "Volume set to " + String(vol));
                } else {
                  request->send(400, "text/plain", "Invalid volume value (0-21)");
                }
              } else {
                request->send(400, "text/plain", "Missing 'value' parameter");
              } });

    server.onNotFound(notFound);
    server.begin();
}
