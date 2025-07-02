#pragma once

#include "Audio.h"

#define I2S_DOUT 47
#define I2S_BCLK 21
#define I2S_LRC 38

Audio audio;

void setupAudio()
{
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
}