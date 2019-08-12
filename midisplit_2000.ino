//Juha Kivekas
//guth.smash@gmail.com
//Use as you please and modify to your hearts extent
//originally written for Look Mum No Computer's gameboy megamachine

//CONNECTIONS
// RX and TX are the MIDI in and outs
// Pin 2 will select between stacking and cycling polyphony modes
// Pin 3 will select between poly and unison modes


//see github for MIDI library installation and usage guide:
//https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>

//use the hardware serial RX and TX pins for MIDI
MIDI_CREATE_DEFAULT_INSTANCE();
#define CHANNELS 6
//this array is used to store the state of what notes are playing on what channels
volatile byte pitches[CHANNELS];
int modeSwitchPin = 2;
int unisonSwitchPin = 3;
int mode, lastmode = -1;
int unison, lastunison = -1;

void splitNoteOnStack(byte channel, byte pitch, byte velocity) {
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

volatile int cycle = 0;
void splitNoteOnCyclic(byte channel, byte pitch, byte velocity) {
  bool found = false;
  int i;
  //find a free/silent voice
  for(i=0; i<CHANNELS; i++){
    cycle = (cycle+1) % CHANNELS;
    if(pitches[cycle] == 0){
      found = true;
      break;
    }
  }
  //set channel note and send the mono note
  if(found){
    pitches[cycle] = pitch;
    MIDI.sendNoteOn(pitch, velocity, cycle+1);
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

void unisonNoteOn(byte channel, byte pitch, byte velocity) {
  int i;
  //send message to all channels
  for(i=0; i<CHANNELS; i++){
    MIDI.sendNoteOn(pitch, velocity, i+1);
  }
}

void unisonNoteOff(byte channel, byte pitch, byte velocity) {
  int i;
  //send message to all channels
  for(i=0; i<CHANNELS; i++){
    MIDI.sendNoteOff(pitch, velocity, i+1);
  }
}

void setup() {
  //set all channels to free/silent
  memset(pitches, 0, CHANNELS);
  
  pinMode(modeSwitchPin, INPUT);

  //connect the MIDI library to our code by callback functions
  //(https://en.wikipedia.org/wiki/Inversion_of_control first paragrapg explains it)
  MIDI.setHandleNoteOff(splitNoteOff);
  MIDI.begin(1);
  MIDI.turnThruOff();
}

void loop() {
  //all code ecexution is driven by the MIDI library that receives MIDI events
  //(https://en.wikipedia.org/wiki/Event-driven_programming)
  MIDI.read();

  //check if mode switches have changed
  mode = digitalRead(modeSwitchPin);
  unison = digitalRead(unisonSwitchPin);
  if(mode == lastmode && unison == lastunison){
    //this is just to avoid setting the HandleNoteOn functions too often, setting them constantly
    //sounds glitch prone, so we just skip the rest of the loop if the switch state hasn't changed
    return;
  }

  //set mode according to switch states
  if(unison == HIGH){
    MIDI.setHandleNoteOn(unisonNoteOn);
    MIDI.setHandleNoteOff(unisonNoteOff);
  }else if(mode == HIGH){
    MIDI.setHandleNoteOn(splitNoteOnCyclic);
    MIDI.setHandleNoteOff(splitNoteOff);
  }else{
    MIDI.setHandleNoteOn(splitNoteOnStack);
    MIDI.setHandleNoteOff(splitNoteOff);
  }
  lastmode = mode;
  lastunison = unison;
}
