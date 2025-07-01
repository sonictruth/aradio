#pragma once
#include <ESPAsyncWebServer.h>
#include "Audio.h"
#include "epromAddreses.h"

AsyncWebServer server(80);
extern Audio audio;

extern char stationName[256];
extern char stationTitle[256];

const char htmlPage[] PROGMEM = R"rawliteral(
    <!DOCTYPE html><html lang=en>
    <meta charset=utf-8><meta name=viewport content="width=device-width, initial-scale=1.0">
    <title>ARadio</title><body>
    <div id=app></div>
    <script type=module src="https://sonictruth.github.io/aradio/web-ui.js"></script>. 
    )rawliteral";

auto escape_csv = [](const char *src, char *dest, size_t destSize)
{
  size_t j = 0;
  for (size_t i = 0; src[i] && j + 1 < destSize; ++i)
  {
    if (src[i] == ',' || src[i] == '"')
    {
      if (j + 2 >= destSize)
        break;
      dest[j++] = '"';
    }
    dest[j++] = src[i];
  }
  dest[j] = '\0';
};

inline void setupWebServer()
{

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
      AsyncWebServerResponse *resp = request->beginResponse(404, "text/plain", "Not found");
      resp->addHeader("Access-Control-Allow-Origin", "*");
      request->send(resp); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", htmlPage); });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
                int volume = audio.getVolume();
                int isRunning = audio.isRunning() ? 1 : 0;
                char escStationName[512];
                char escStationTitle[512];

                escape_csv(stationName, escStationName, sizeof(escStationName));
                escape_csv(stationTitle, escStationTitle, sizeof(escStationTitle));

                char response[1050];
                snprintf(response, sizeof(response), "%d,%d,%s,%s", isRunning ,volume, escStationName, escStationTitle);

                AsyncWebServerResponse *resp = request->beginResponse(200, "text/plain", response);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                request->send(resp); });

  server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              stationName[0] = '\0';
              stationTitle[0] = '\0';
              int code = 200;
              String responseBody = "ok";
              if (request->hasParam("url"))
              {
                String streamURL = request->getParam("url")->value();
                if (audio.isRunning())
                {
                  audio.stopSong();
                  delay(2000);
                }
                if (audio.connecttohost(streamURL.c_str()))
                {
                  code = 200;
                  responseBody = "Playing: " + streamURL;
                  EEPROM.writeString(LAST_URL_EPROM_ADDEESS, streamURL);
                  EEPROM.commit();
                }
                else
                {
                  code = 500;
                  responseBody = "Failed to connect to: " + streamURL;
                }
              }
              else
              {
                code = 400;
                responseBody = "Missing 'url' parameter";
              }
              AsyncWebServerResponse *resp = request->beginResponse(code, "text/plain", responseBody);
              resp->addHeader("Access-Control-Allow-Origin", "*");
              request->send(resp); });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
               stationName[0] = '\0';
              stationTitle[0] = '\0';
              int code = 200;
              String responseBody = "";
              if (!audio.isRunning())
              {
                code = 400;
                responseBody = "Not playing any stream";
              }
              else
              {
                audio.stopSong();
                EEPROM.writeString(LAST_URL_EPROM_ADDEESS, "");
                EEPROM.commit();
                code = 200;
                responseBody = "Stream stopped";
              }
              AsyncWebServerResponse *resp = request->beginResponse(code, "text/plain", responseBody);
              resp->addHeader("Access-Control-Allow-Origin", "*");
              request->send(resp); });

  server.on("/setvolume", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              int code = 200;
              String responseBody = "";
              if (request->hasParam("value")) {
                String valueStr = request->getParam("value")->value();
                int vol = valueStr.toInt();
                if (vol >= 0 && vol <= 21) {
                  audio.setVolume(vol);
               
                  EEPROM.writeInt(VOLUME_EPROM_ADDRESS, vol); 
                  EEPROM.commit();
                  code = 200;
                  responseBody = "Volume set to " + String(vol);
                } else {
                  code = 400;
                  responseBody = "Volume must be between 0 and 21";
                }
              } else {
                code = 400;
                responseBody = "Missing 'value' parameter";
              } 
              AsyncWebServerResponse *resp = request->beginResponse(code, "text/plain", responseBody);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                request->send(resp); });

  server.begin();
}
