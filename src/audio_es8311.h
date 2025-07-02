#pragma once

#include "Audio.h"
#include "es8311.h"

#define I2S_DOUT 8
#define I2S_BCLK 9
#define I2S_MCLK 16
#define I2S_LRC 45
#define PA_ENABLE 46
#define ES_CODEC_I2C_SCL 14
#define ES_CODEC_I2C_SDA 15

Audio audio;
ES8311 es;

void setupAudio()
{
  // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
  pinMode(PA_ENABLE, OUTPUT);
  digitalWrite(PA_ENABLE, HIGH);
  if (!es.begin(ES_CODEC_I2C_SDA, ES_CODEC_I2C_SCL, 400000))
    log_e("ES8311 begin failed");
  es.setVolume(70);
  es.setBitsPerSample(16);
}