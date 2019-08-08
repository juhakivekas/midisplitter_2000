//Juha Kivekas
//guth.smash@gmail.com
//Use as you please and modify to your hearts extent
//originally written for Look Mum No Computer's gameboy megamachine

//see github for MIDI library installation and usage guide:
//https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>

//use the hardware serial RX and TX pins for MIDI
MIDI_CREATE_DEFAULT_INSTANCE();

//since this splits polyphony to different MIDI channels, we have max 16 voices
#define CHANNELS 16
//this array is used to store the state of what notes are playing on what channels
volatile byte pitches[CHANNELS];

void splitNoteOn(byte channel, byte pitch, byte velocity) {
  bool found = false;
  int i;
  //find a free/silent voice
  for(i=0; i<CHANNELS; i++){
    if(pitches[i] == 0){
      found = true;
      break;
    }
  }
  //set channel note and send the mono note
  if(found){
    pitches[i] = pitch;
    MIDI.sendNoteOn(pitch, velocity, i+1);
  }
  //if there is no free channel found, events are dropped
}

void splitNoteOff(byte channel, byte pitch, byte velocity) {
  bool found = false;
  int i;
  //find voice with the incoming pitch
  for(i=0; i<CHANNELS; i++){
    if(pitches[i] == pitch){
      found = true;
      break;
    }
  }
  //set channel note off
  if(found){
    pitches[i] = 0;
    MIDI.sendNoteOff(pitch, velocity, i+1);
  }
  //if the pitch wasn't found on any channel, then events are dropped
}

void setup() {
  //set all channels to free/silent
  memset(pitches, 0, CHANNELS);
  
  //connect the MIDI library to our code by callback functions
  //(https://en.wikipedia.org/wiki/Inversion_of_control first paragrapg explains it)
  MIDI.setHandleNoteOn(splitNoteOn);
  MIDI.setHandleNoteOff(splitNoteOff);
  MIDI.begin(1);
  MIDI.turnThruOff();
}

void loop() {
  //all code ecexution is driven by the MIDI library that receives MIDI events
  //(https://en.wikipedia.org/wiki/Event-driven_programming)
  MIDI.read();
}
