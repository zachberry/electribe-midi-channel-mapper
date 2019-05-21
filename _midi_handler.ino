#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.

#include "_midi_handler.h"

#define SOFTWARE_VERSION 1

// Number of seconds to hold down save button on power up to enter
// "all outputs disabled" mode
#define DISABLE_OUTPUTS_DELAY 4

// Potentiometer inputs
#define MAX_POT_VALUE 1024
#define TOTAL_POT_STEPS 17
#define CHAN_IN_POT 2
#define CHAN_OUT_POT 3

float EMA_a = 0.6; //initialization of EMA alpha
int EMA_S_in = 0;  //initialization of EMA S for input channel
int EMA_S_out = 0; //initialization of EMA S for output channel

#define DEBUG true

byte in_pot_channel = 0;
byte out_pot_channel = 0;
byte last_in_pot_channel = 0;
byte last_out_pot_channel = 0;
bool is_out_pot_dirty = false;

// Stuff that only runs every so often
#define LOOP_DELAY 500
int loop_ticks = 0;

// Button
#define BUTTON_PIN 4
int button_state;

void setup()
{
	LCD::init();

	// Save Button
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	button_state = digitalRead(BUTTON_PIN);

	byte channelMap = EEPROM.init(SOFTWARE_VERSION, defaultChannelMap, button_state == LOW);
	ChannelMap::init(channelMap);

	LCD::displayZeros();

	int counter = 0;
	while (button_state == LOW)
	{
		if (counter > (DISABLE_OUTPUTS_DELAY * 10))
		{
			LCD::displayAllDashes();
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
		ChannelMap::disableAllChannels();
		EEPROM.put(CHANNEL_MAP_ADDR, channel_map);
	}

	// Display Zeroes

	LCD::displayAllZeroes();
	delay(1000);

	MIDIHandler::init();

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
		// resetActiveStateForAllChannels();
		MIDIHandler::resetActiveState();

		loop_ticks = 0;
	}
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
	else if (is_out_pot_dirty && out_pot_channel == ChannelMap::getOutputChannel())
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
	bool isOmni = ChannelMap::isOMNIMode();

	// Display the colons if in omni mode to indicate locked outputs
	LCD::writeColon(isOmni);

	// We display the left dot if we got a MIDI message on that channel:
	LCD::writeInputChannel(ChannelMap::inputChannel, MIDIHandler::isChannelActive(ChannelMap::inputChannel));

	// If the out pot has been rotated (is_out_pot_dirty=true) then we display
	// the current actual out pot value along with the 'dirty' dot.
	// If we're in omni mode, ignore this case.
	if (is_out_pot_dirty && (!isOmni || in_pot_channel == 0))
	{
		LCD::writeOutputChannel(out_pot_channel, true);
	}
	// Otherwise, show what the current mapping is for the given in pot channel.
	else
	{
		LCD::writeOutputChannel(ChannelMap::getOutputChannel(), false);
	}
}

void updateChannels()
{
	if (!is_out_pot_dirty)
	{
		return;
	}

	// channel_map[in_pot_channel] = out_pot_channel;
	ChannelMap::setOutputChannel(in_pot_channel, out_pot_channel);
	EEPROM.write(CHANNEL_MAP_ADDR + in_pot_channel, out_pot_channel);

	is_out_pot_dirty = false;
}
