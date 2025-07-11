/*
	ES8311 - An ES8311 Codec driver library for Arduino

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	examples:

	//one I2C bus: (default behaviour)
	ES8311 es;
	es.begin(sda, scl);

	//two I2C busses:
	TwoWire i2cBusOne = TwoWire(0);
	TwoWire i2cBusTwo = TwoWire(1);
    ES8311 es(&i2cBusOne);

    i2cBusOne.begin(sda, scl, 400000);
*/

#include "es8311.h"

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
    /*!<mclk     rate   pre_div  mult  adc_div dac_div fs_mode lrch  lrcl  bckdiv osr */
    /* 8k */
    {12288000, 8000, 0x06, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 8000, 0x03, 0x01, 0x03, 0x03, 0x00, 0x05, 0xff, 0x18, 0x10, 0x10},
    {16384000, 8000, 0x08, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000, 8000, 0x04, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 8000, 0x03, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000, 8000, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 8000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000, 8000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 8000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000, 8000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 11.025k */
    {11289600, 11025, 0x04, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800, 11025, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400, 11025, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200, 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 12k */
    {12288000, 12000, 0x04, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 12000, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 12000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 16k */
    {12288000, 16000, 0x03, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 16000, 0x03, 0x01, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 16000, 0x04, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000, 16000, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 16000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000, 16000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 16000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000, 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 16000, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000, 16000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 22.05k */
    {11289600, 22050, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800, 22050, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400, 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200, 22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {705600, 22050, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 24k */
    {12288000, 24000, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 24000, 0x03, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 24000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 24000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 32k */
    {12288000, 32000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 32000, 0x03, 0x02, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 32000, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000, 32000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 32000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000, 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 32000, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000, 32000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 32000, 0x03, 0x03, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
    {1024000, 32000, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 44.1k */
    {11289600, 44100, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400, 44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200, 44100, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 48k */
    {12288000, 48000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 48000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 48000, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 64k */
    {12288000, 64000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 64000, 0x03, 0x02, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {16384000, 64000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 64000, 0x01, 0x02, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {4096000, 64000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 64000, 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {2048000, 64000, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 64000, 0x01, 0x03, 0x01, 0x01, 0x01, 0x00, 0xbf, 0x03, 0x18, 0x18},
    {1024000, 64000, 0x01, 0x03, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 88.2k */
    {11289600, 88200, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800, 88200, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400, 88200, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200, 88200, 0x01, 0x03, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 96k */
    {12288000, 96000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 96000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000, 96000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000, 96000, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000, 96000, 0x01, 0x03, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
};

ES8311::ES8311(TwoWire *TwoWireInstance){
    _TwoWireInstance = TwoWireInstance;
}

ES8311::~ES8311(){
    if (_TwoWireInstance != NULL) {
        _TwoWireInstance->end();
    }
}

/*
* look for the coefficient in coeff_div[] table
*/
int ES8311::get_coeff(uint32_t mclk, uint32_t rate){
    for (int i = 0; i < (sizeof(coeff_div) / sizeof(coeff_div[0])); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk) {
            return i;
        }
    }
    return -1;
}

bool ES8311::begin(int32_t sda, int32_t scl, uint32_t frequency) {
    bool ok = true;
    uint8_t reg = 0;

    if((sda >= 0) && (scl >= 0)){
	    ok = _TwoWireInstance->begin(sda, scl, frequency);
        _TwoWireInstance->beginTransmission(ES8311_ADDR);
        ok = (Wire.endTransmission() == 0);
        if(!ok) {
            _TwoWireInstance->end();
            log_e("ES8311 not found"); return false;
        }
    }
    else {
        log_e("Invalid SDA/SCL pins");
        return false;
    }

    ok |= WriteReg(0x00, 0x1F);  // Reset
    vTaskDelay(20 / portTICK_PERIOD_MS);
    ok |= WriteReg(0x00, 0x00);  // Release reset
    ok |= WriteReg(0x00, 0x80);  // Power on

    ok |= WriteReg(0x01, 0x3F);  // Enable all clocks

    reg = ReadReg(0x06);
    reg &= ~BIT(5); // SCLK (BCLK) pin not inverted
    ok |= WriteReg(0x06, reg);   //

    ok |= setSampleRate(ES8311_SAMPLE_RATE48);         // default
    ok |= setBitsPerSample(ES8311_BITS_PER_SAMPLE16);  // default

    ok |= WriteReg(0x0D, 0x01); // Power up analog circuitry
    ok |= WriteReg(0x0E, 0x02); // Enable analog PGA, enable ADC modulator
    ok |= WriteReg(0x12, 0x00); // Power-up DAC
    ok |= WriteReg(0x13, 0x10); // Enable output to HP drive
    ok |= WriteReg(0x1C, 0x6A); // ADC Equalizer bypass, cancel DC offset in digital domain
    ok |= WriteReg(0x37, 0x08); // Bypass DAC equalizer

    return ok;
}

bool ES8311::setVolume(uint8_t volume){ // 0...100
    if (volume > 100) {volume = 100;}
    int reg32;
    if (volume == 0) {reg32 = 0;}
    else {            reg32 = ((volume) * 256 / 100) - 1;}
    return WriteReg(0x32, reg32);
}

uint8_t ES8311::getVolume(){
    uint8_t reg32 = ReadReg(0x32);
    uint8_t volume;
    if (reg32 == 0) {
        volume = 0;
    } else {
        volume = ((reg32 * 100) / 256) + 1;
    }
    return volume;
}

bool ES8311::setSampleRate(uint32_t sample_rate){
    uint8_t reg = 0;
    bool ok = true;
    _mclk_hz = sample_rate * 256; // default MCLK frequency
    if(sample_rate > 64000) _mclk_hz /= 2;
    int coeff = get_coeff(_mclk_hz, sample_rate);
    if (coeff < 0) {log_e("Invalid sample rate %i", sample_rate); return false;}
    const struct _coeff_div *const selected_coeff = &coeff_div[coeff];
    reg = ReadReg(0x02);
    reg |= (selected_coeff->pre_div - 1) << 5;
    reg |= selected_coeff->pre_multi << 3;
    ok |= WriteReg(0x02, reg); // Set pre_div and pre_multi
    const uint8_t reg03 = (selected_coeff->fs_mode << 6) | selected_coeff->adc_osr;
    ok |= WriteReg(0x03, reg03); // Set fs_mode and adc_osr
    ok |= WriteReg(0x04, selected_coeff->dac_osr); // Set dac_osr
    const uint8_t reg05 = ((selected_coeff->adc_div - 1) << 4) | (selected_coeff->dac_div - 1);
    ok |= WriteReg(0x05, reg05); // Set adc_div and dac_div
    reg = ReadReg(0x06);
    reg &= 0xE0;
    if (selected_coeff->bclk_div < 19) {reg |= (selected_coeff->bclk_div - 1) << 0;}
    else {                              reg |= (selected_coeff->bclk_div) << 0;}
    ok |= WriteReg(0x06, reg); // Set bclk_div
    reg = ReadReg(0x07);
    reg &= 0xC0;
    reg |= selected_coeff->lrck_h << 0;
    ok |= WriteReg(0x07, reg); // Set lrck_h
    ok |= WriteReg(0x08, selected_coeff->lrck_l); // Set lrck_l
    return ok;
}

bool ES8311::setBitsPerSample(uint8_t bps){
    uint8_t reg09 = ReadReg(0x09);
    uint8_t reg0A = ReadReg(0x0A);
    switch (bps) {
        case 16: reg09 |= (3 << 2); reg0A |= (3 << 2); break;
        case 18: reg09 |= (2 << 2); reg0A |= (2 << 2); break;
        case 20: reg09 |= (1 << 2); reg0A |= (1 << 2); break;
        case 24: reg09 |= (0 << 2); reg0A |= (0 << 2); break;
        case 32: reg09 |= (4 << 2); reg0A |= (4 << 2); break;
        default: return false; // Invalid bits per sample
    }
    bool ok = WriteReg(0x09, reg09);
    ok |= WriteReg(0x0A, reg0A);
    return ok;
}

bool ES8311::enableMicrophone(bool enable){
    uint8_t reg = 0x1A; // enable analog MIC and max PGA gain
    if (enable) {
        reg |= BIT(6);
    }
    bool ok = WriteReg(0x17, 0xC8); // ADC_VOLUME
    ok |= WriteReg(0x14, reg); // Enable MIC
    return ok;
}

bool ES8311::setMicrophoneGain(uint8_t gain){ // 0...7
    uint8_t reg = ReadReg(0x16);
    reg &= 0xF8; // Clear gain bits
    if (gain > 7) {gain = 7;}
    reg |= gain; // Set gain bits
    bool ok = WriteReg(0x16, gain); // ADC_VOLUME
    return ok;
}

uint8_t ES8311::getMicrophoneGain(){
    uint8_t reg = ReadReg(0x16);
    return (reg & 0x07); // Get gain bits
}

bool ES8311::WriteReg(uint8_t reg, uint8_t val){
	_TwoWireInstance->beginTransmission(ES8311_ADDR);
	_TwoWireInstance->write(reg);
	_TwoWireInstance->write(val);
	return _TwoWireInstance->endTransmission() == 0;
}

uint8_t ES8311::ReadReg(uint8_t reg){
	_TwoWireInstance->beginTransmission(ES8311_ADDR);
	_TwoWireInstance->write(reg);
	_TwoWireInstance->endTransmission(false);

	uint8_t val = 0u;
	_TwoWireInstance->requestFrom(uint16_t(ES8311_ADDR), (uint8_t)1, true);
	if(_TwoWireInstance->available() >= 1){
        val = _TwoWireInstance->read();
	}
    _TwoWireInstance->endTransmission();
	return val;
}

void ES8311::read_all(){
    for (uint8_t i = 0; i < 0x4A; i++) {
        Serial.printf("0x%02X: 0x%02X\n", i, ReadReg(i));
    }
}
