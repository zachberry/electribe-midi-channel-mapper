#include <EEPROM.h>
#include <MIDI.h>
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

#define SOFTWARE_VERSION 1

// Number of seconds to hold down save button on power up to enter
// "all outputs disabled" mode
#define DISABLE_OUTPUTS_DELAY 4

// Matrix raw display values
#define MATRIX_BLANK 0B000000000
#define MATRIX_DASH 0B001000000

// Potentiometer inputs
#define MAX_POT_VALUE 1024
#define TOTAL_POT_STEPS 17
#define CHAN_IN_POT 2
#define CHAN_OUT_POT 3

float EMA_a = 0.6; //initialization of EMA alpha
int EMA_S_in = 0;  //initialization of EMA S for input channel
int EMA_S_out = 0; //initialization of EMA S for output channel

#define DEBUG true

MIDI_CREATE_DEFAULT_INSTANCE();

byte in_pot_channel = 0;
byte out_pot_channel = 0;
byte last_in_pot_channel = 0;
byte last_out_pot_channel = 0;
bool is_out_pot_dirty = false;

// Array to hold inputs and outputs.  Index is input channel, value is output channel.
// 0 = No output, 1 = Midi Channel 1, ..., 16 = Midi Channel 16
byte channel_map[17] = {MIDI_CHANNEL_OFF, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// Array to keep track of which channels are active. We use this information to display
// a dot in the LED display next to the input channel (so the user can see visually
// which channels are getting messages)
bool active_map[17] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

// EEPROM Addresses
#define VERSION_ADDR 0     // value of 1 if memory has been initialized
#define CHANNEL_MAP_ADDR 1 // 1 - 17

// Stuff that only runs every so often
#define LOOP_DELAY 500
int loop_ticks = 0;

// Button
#define BUTTON_PIN 4
int button_state;

void setup()
{
  // Save Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button_state = digitalRead(BUTTON_PIN);

  // Load saved data from EEPROM if it has been initialized
  byte versionNum = EEPROM.read(VERSION_ADDR);
  if ((versionNum == SOFTWARE_VERSION) && (button_state == HIGH))
  {
    EEPROM.get(CHANNEL_MAP_ADDR, channel_map);
  }
  // Otherwise, initialize the EEPROM
  else
  {
    EEPROM.put(CHANNEL_MAP_ADDR, channel_map);

    // Set the initialized flag
    EEPROM.write(VERSION_ADDR, 1);
  }

  // Init display
  matrix.begin(0x70);
  matrix.drawColon(false);
  matrix.writeDigitRaw(0, MATRIX_BLANK);
  matrix.writeDigitRaw(1, MATRIX_BLANK);
  matrix.writeDigitRaw(3, MATRIX_BLANK);
  matrix.writeDigitRaw(4, MATRIX_BLANK);
  matrix.writeDisplay();

  int counter = 0;
  while (button_state == LOW)
  {
    if (counter > (DISABLE_OUTPUTS_DELAY * 10))
    {
      matrix.writeDigitRaw(0, MATRIX_DASH);
      matrix.writeDigitRaw(1, MATRIX_DASH);
      matrix.writeDigitRaw(3, MATRIX_DASH);
      matrix.writeDigitRaw(4, MATRIX_DASH);
      matrix.writeDisplay();
    }
    else
    {
      counter++;
    }
    button_state = digitalRead(BUTTON_PIN);
    delay(100);
  }

  // If the user held the button long enough, disable all channels
  if (counter > (DISABLE_OUTPUTS_DELAY * 10))
  {
    disableAllChannels();
  }

  // Display Zeroes

  matrix.setBrightness(1);
  matrix.writeDigitNum(0, 0, false);
  matrix.writeDigitNum(1, 0, false);
  matrix.writeDigitNum(3, 0, false);
  matrix.writeDigitNum(4, 0, false);
  matrix.writeDisplay();
  delay(1000);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);
  MIDI.setHandleAfterTouchPoly(handleAfterTouchPolyPressure);
  MIDI.setHandleAfterTouchChannel(handleAfterTouchChannelPressure);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.setHandleTimeCodeQuarterFrame(handleTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(handleSongPosition);
  MIDI.setHandleSongSelect(handleSongSelect);
  MIDI.setHandleTuneRequest(handleTuneRequest);
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleStart(handleStart);
  MIDI.setHandleContinue(handleContinue);
  MIDI.setHandleStop(handleStop);
  MIDI.setHandleActiveSensing(handleActiveSensing);
  MIDI.setHandleSystemReset(handleSystemReset);
  // (SysEx messages are not supported)

  MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.turnThruOff();

  getPotInputs();
  displayChannels();
}

void loop()
{
  MIDI.read();

  button_state = digitalRead(BUTTON_PIN);
  if (button_state == LOW)
  {
    updateChannels();
  }

  // Run the pot inputs and display channels every 100 loops
  if (++loop_ticks == LOOP_DELAY)
  {
    displayChannels();
    getPotInputs();
    resetActiveStateForAllChannels();

    loop_ticks = 0;
  }
}

byte getOutChannel(byte in_channel)
{
  // The OMNI channel output takes first prioirty. If it's set
  // then send ALL messages to that channel!
  if (isOMNIMode())
  {
    return channel_map[MIDI_CHANNEL_OMNI];
  }

  return channel_map[in_channel];
}

bool isOMNIMode()
{
  return channel_map[MIDI_CHANNEL_OMNI] != MIDI_CHANNEL_OFF;
}

// -------------------------------------------
// MIDI Handlers:
// -------------------------------------------

void handleProgramChange(byte channel, byte number)
{
  markChannelAsActive(channel);
  MIDI.sendProgramChange(number, getOutChannel(channel));
}

void handleNoteOn(byte channel, byte note, byte velocity)
{
  markChannelAsActive(channel);
  MIDI.sendNoteOn(note, velocity, getOutChannel(channel));
}

void handleNoteOff(byte channel, byte note, byte velocity)
{
  markChannelAsActive(channel);
  MIDI.sendNoteOff(note, velocity, getOutChannel(channel));
}

void handlePitchBend(byte channel, int bend)
{
  markChannelAsActive(channel);
  MIDI.sendPitchBend(bend, getOutChannel(channel));
}

void handleControlChange(byte channel, byte number, byte value)
{
  markChannelAsActive(channel);
  MIDI.sendControlChange(number, value, getOutChannel(channel));
}

void handleAfterTouchPolyPressure(byte channel, byte note, byte pressure)
{
  markChannelAsActive(channel);
  MIDI.sendAfterTouch(note, pressure, getOutChannel(channel));
}

void handleAfterTouchChannelPressure(byte channel, byte pressure)
{
  markChannelAsActive(channel);
  MIDI.sendAfterTouch(pressure, getOutChannel(channel));
}

void handleTimeCodeQuarterFrame(byte data)
{
  MIDI.sendTimeCodeQuarterFrame(data);
}

void handleSongPosition(unsigned beats)
{
  MIDI.sendSongPosition(beats);
}

void handleSongSelect(byte songnumber)
{
  MIDI.sendSongSelect(songnumber);
}

void handleTuneRequest()
{
  MIDI.sendTuneRequest();
}

// Real time messages (Messages that don't specify channels)
// (These messages don't have their own sendX method, instead we must use a
// lower-level send method from the library):
void handleClock()
{
  MIDI.sendRealTime(midi::Clock);
}

void handleStart()
{
  MIDI.sendRealTime(midi::Start);
}

void handleContinue()
{
  MIDI.sendRealTime(midi::Continue);
}

void handleStop()
{
  MIDI.sendRealTime(midi::Stop);
}

void handleActiveSensing()
{
  MIDI.sendRealTime(midi::ActiveSensing);
}

void handleSystemReset()
{
  MIDI.sendRealTime(midi::SystemReset);
}

// -------------------------------------------
// Manual input methods:
// -------------------------------------------

void getPotInputs()
{
  // In pot should read OMNI (0), then 1 to 16
  in_pot_channel = normalizePotInput(emaSmooth(analogRead(CHAN_IN_POT), &EMA_S_in), true);
  // Out pot should read OFF, then 1 to 16
  out_pot_channel = normalizePotInput(emaSmooth(analogRead(CHAN_OUT_POT), &EMA_S_out), false);

  // If the in pot has changed then reset the out pot dirty state
  // (in other words, any un-saved out pot value is discarded)
  if (is_out_pot_dirty && in_pot_channel != last_in_pot_channel)
  {
    is_out_pot_dirty = false;
  }
  // If the out pot has changed we set the dirty flag, meaning the
  // user has rotated the pot and is wanting to set the out channel
  else if (!is_out_pot_dirty && out_pot_channel != last_out_pot_channel)
  {
    is_out_pot_dirty = true;
  }
  // If the out pot has changed back to the currently set value
  // of the input, the out pot is not dirty anymore
  else if (is_out_pot_dirty && out_pot_channel == channel_map[in_pot_channel])
  {
    is_out_pot_dirty = false;
  }

  last_in_pot_channel = in_pot_channel;
  last_out_pot_channel = out_pot_channel;
}

int normalizePotInput(float rawIn, bool zeroIndex)
{
  rawIn = rawIn / MAX_POT_VALUE;
  rawIn = rawIn * (TOTAL_POT_STEPS - 1);
  int out = (int)(rawIn + 0.5f);

  // Putting this here because the channel input starts at zero,
  // but the channel output starts at 1 and goes to "--"
  if (!zeroIndex)
  {
    out += 1;
  }
  return out;
}

int emaSmooth(int sensorValue, int *EMA_S)
{
  *EMA_S = (EMA_a * sensorValue) + ((1 - EMA_a) * (*EMA_S));
  return *EMA_S;
}

// -------------------------------------------
// Display methods:
// -------------------------------------------

void displayChannels()
{
  bool isOmni = isOMNIMode();

  // Display the colons if in omni mode to indicate locked outputs
  matrix.drawColon(isOmni);

  // We display the left dot if we got a MIDI message on that channel:
  displayLeft(in_pot_channel, isChannelActive(in_pot_channel));

  // If the out pot has been rotated (is_out_pot_dirty=true) then we display
  // the current actual out pot value along with the 'dirty' dot.
  // If we're in omni mode, ignore this case.
  if (is_out_pot_dirty && (!isOmni || in_pot_channel == 0))
  {
    displayRight(out_pot_channel, true);
  }
  // Otherwise, show what the current mapping is for the given in pot channel.
  else
  {
    displayRight(getOutChannel(in_pot_channel), false);
  }
}

void displayLeft(byte val, bool dot)
{
  int leftDigit = val / 10;
  if (leftDigit == 0)
  {
    matrix.writeDigitRaw(0, MATRIX_BLANK);
  }
  else
  {
    matrix.writeDigitNum(0, (val / 10), false);
  }
  matrix.writeDigitNum(1, val % 10, dot);
  matrix.writeDisplay();
}

// NOTE: MIDI_CHANNEL_OFF is actually 17, but we want to display that
// on the LCD as "--".
void displayRight(int val, bool dot)
{
  if (val == MIDI_CHANNEL_OFF)
  {
    // Display "--"
    matrix.writeDigitRaw(3, MATRIX_DASH);
    matrix.writeDigitRaw(4, MATRIX_DASH);
  }
  else
  {
    int leftDigit = val / 10;
    if (leftDigit == 0)
    {
      matrix.writeDigitRaw(3, MATRIX_BLANK);
    }
    else
    {
      matrix.writeDigitNum(3, (val / 10), false);
    }
    matrix.writeDigitNum(4, val % 10, dot);
  }

  matrix.writeDisplay();
}

void updateChannels()
{
  // If we're in omni mode, ignore all changes except for 0
  if ((!isOMNIMode() || in_pot_channel == 0) && is_out_pot_dirty)
  {
    channel_map[in_pot_channel] = out_pot_channel;
    EEPROM.write(CHANNEL_MAP_ADDR + in_pot_channel, out_pot_channel);

    is_out_pot_dirty = false;
  }
}

void disableAllChannels()
{
  channel_map[MIDI_CHANNEL_OMNI] = MIDI_CHANNEL_OFF;
  channel_map[1] = MIDI_CHANNEL_OFF;
  channel_map[2] = MIDI_CHANNEL_OFF;
  channel_map[3] = MIDI_CHANNEL_OFF;
  channel_map[4] = MIDI_CHANNEL_OFF;
  channel_map[5] = MIDI_CHANNEL_OFF;
  channel_map[6] = MIDI_CHANNEL_OFF;
  channel_map[7] = MIDI_CHANNEL_OFF;
  channel_map[8] = MIDI_CHANNEL_OFF;
  channel_map[9] = MIDI_CHANNEL_OFF;
  channel_map[10] = MIDI_CHANNEL_OFF;
  channel_map[11] = MIDI_CHANNEL_OFF;
  channel_map[12] = MIDI_CHANNEL_OFF;
  channel_map[13] = MIDI_CHANNEL_OFF;
  channel_map[14] = MIDI_CHANNEL_OFF;
  channel_map[15] = MIDI_CHANNEL_OFF;
  channel_map[16] = MIDI_CHANNEL_OFF;
  EEPROM.put(CHANNEL_MAP_ADDR, channel_map);
}

void markChannelAsActive(byte channel)
{
  active_map[MIDI_CHANNEL_OMNI] = true;

  active_map[channel] = true;
}

bool isChannelActive(byte channel)
{
  return active_map[channel];
}

void resetActiveStateForAllChannels()
{
  active_map[MIDI_CHANNEL_OMNI] = false;
  active_map[1] = false;
  active_map[2] = false;
  active_map[3] = false;
  active_map[4] = false;
  active_map[5] = false;
  active_map[6] = false;
  active_map[7] = false;
  active_map[8] = false;
  active_map[9] = false;
  active_map[10] = false;
  active_map[11] = false;
  active_map[12] = false;
  active_map[13] = false;
  active_map[14] = false;
  active_map[15] = false;
  active_map[16] = false;
}
