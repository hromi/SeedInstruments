// Title: Polyphonic Conductive Harp
// Hardware: Daisy Seed / MPR121
// Author: Daniel Devatman Hromada
// Acknowledgments: Thanks for prof. Berit Greinke for combing hair instrument idea and bringing conductive fibers into the house.

#include "DaisyDuino.h"
DaisyHardware hw;
TwoWire wire1;

size_t num_channels;

static Tone flt;
static Oscillator osc[12];
static Oscillator halbtones[5];



//C4 scale - shuffle array indices according to spatial configuration of Your conductive keys/conductive fibers on Your instrument
const float scale[12] = {293.66,329.63,349.23,392.00,440.0,523.25,0,493.8,0,0,0,261.63};
const float halb_scale[5] = {277.18,311.13,369.99,415.3,466.16};

//Ode for Joy
//const uint8_t notes[30] = {2,2,3,4,4,3,2,1,0,0,1,2,2,1,1,2,2,3,4,4,3,2,1,0,0,1,2,1,0,0};
//const uint8_t lengths[30]= {2,2,2,2,2,2,2,2,2,2,2,2,3,1,4,2,2,2,2,2,2,2,2,2,2,2,2,3,1,4};

/* base for polyphonic one ;)
bool silences[6] = {true,true,true,true,true,true};
Oscillator oscz[6];
*/

uint8_t idx=0;
bool silence=true;
bool octave_high=false;


#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;



Adsr env;
Metro tick;
bool gate;

void MyCallback(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float osc_out=0;
    int strings=0;
    if (currtouched==0) { 
      out[0][i]=0;
      out[1][i]=0;
    }
    else {
      if (currtouched==2049) {
        osc_out+=halbtones[0].Process();
      }
      else if (currtouched==6) {
        osc_out+=halbtones[1].Process();
      }
      else if (currtouched==12) {
        osc_out+=halbtones[2].Process();
      }
      else if (currtouched==24) {
        osc_out+=halbtones[3].Process();
      }
      else if (currtouched==144) {
        osc_out+=halbtones[4].Process();
      }
      else {
        for (uint8_t osc_id=0; osc_id<12; osc_id++) {
          if ((currtouched & _BV(osc_id))  ) {
            osc_out+=osc[osc_id].Process();
            //strings++;
          }
        }
      }
        for (size_t chn = 0; chn < num_channels; chn++) {
              out[chn][i] = osc_out;
        }
    }
  }
}

void setup() {
  float samplerate;
  //specify other touch/release thresholds, wet Your fingers or Your fibers or use Your tongue ;)
  //if (!cap.begin(0x5A,&wire1,18,6)) {
  if (!cap.begin(0x5A,&wire1)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!"); 
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  samplerate = DAISY.get_samplerate();

  env.Init(samplerate);
  //osc.Init(samplerate);

  // Set up metro to pulse every second
  tick.Init(0.1, samplerate);

  for (uint8_t i=0; i<12; i++) {
    osc[i].Init(samplerate);
    osc[i].SetFreq(scale[i]);
    osc[i].SetWaveform(osc[i].WAVE_SIN);
    osc[i].SetAmp(0.1);
  }
  for (uint8_t i=0; i<5; i++) {
    halbtones[i].Init(samplerate);
    halbtones[i].SetFreq(halb_scale[i]);
    halbtones[i].SetWaveform(halbtones[i].WAVE_SIN);
    halbtones[i].SetAmp(0.1);
  }

  DAISY.begin(MyCallback);
}

void loop() {
  currtouched = cap.touched();
  Serial.println(currtouched);
  //lasttouched = currtouched;
  if (currtouched==64) {
    //touch the last button for 100ms to flip the octave
    delay(200);
    currtouched = cap.touched();
    if (currtouched==64) {
        if (octave_high) {
          Serial.println("descending to 4th octave");
          for (uint8_t i=0; i<12; i++) {
            osc[i].SetFreq(scale[i]);
          }
          for (uint8_t i=0; i<5; i++) {
            halbtones[i].SetFreq(halb_scale[i]);
          }
          octave_high=false;
        } else {
          Serial.println("ascending to 5th octave");
          for (uint8_t i=0; i<12; i++) {
            osc[i].SetFreq(scale[i]*2);
          }
          for (uint8_t i=0; i<5; i++) {
            halbtones[i].SetFreq(halb_scale[i]*2);
          }
          octave_high=true;
        }
        }
    }
  
  delay(50);
  
}
