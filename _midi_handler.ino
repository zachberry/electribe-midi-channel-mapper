#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.

#include "_midi_handler.h"

#define SOFTWARE_VERSION 1

// Number of seconds to hold down save button on power up to enter
// "all outputs disabled" mode
#define DISABLE_OUTPUTS_DELAY 4

#define DEBUG true

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
	Hardware::init();

	Storage::init(SOFTWARE_VERSION, ChannelMap::defaultMap, false);
	// byte channelMap = EEPROM.init(SOFTWARE_VERSION, defaultChannelMap, button_state == LOW);
	ChannelMap::init(Storage::getChannelMap());

	LCD::displayAllZeros();

	int counter = 0;
	while (Hardware::isButtonPressed())
	{
		if (counter > (DISABLE_OUTPUTS_DELAY * 10))
		{
			LCD::displayAllDashes();
		}
		else
		{
			counter++;
		}
		// button_state = digitalRead(BUTTON_PIN);
		Hardware::readButton();
		delay(100);
	}

	// If the user held the button long enough, disable all channels
	if (counter > (DISABLE_OUTPUTS_DELAY * 10))
	{
		ChannelMap::disableAllChannels();
		// EEPROM.put(CHANNEL_MAP_ADDR, channel_map);
		Storage.writeChannelMap(ChannelMap::map);
	}

	// Display Zeroes

	LCD::displayAllZeroes();
	delay(1000);

	MIDIHandler::init();

	update();
}

void loop()
{
	// MIDI messages must update on every loop:
	MIDIHandler::update();

	// The LCD and mappings update every LOOP_DELAY ticks:
	if (shouldUpdate())
	{
		update();
	}
}

bool shouldUpdate()
{
	loop_ticks++;

	if (loop_ticks >= LOOP_DELAY)
	{
		loop_ticks = 0;
		return true;
	}

	return false;
}

// Reads hardware, updates the LCD, saves channels if button is depressed
void update()
{
	Hardware::read();

	bool isOmni = ChannelMap::isOMNIMode();
	byte in = Hardware::inputChannel;
	byte out = Hardware::outputChannel;

	// Display the colons if in omni mode to indicate locked outputs
	LCD::setColon(isOmni);

	// We display the left dot if we got a MIDI message on that channel:
	LCD::updateInputChannel(in, MIDIHandler::isChannelActive(in));

	// If the out pot has been rotated (is_out_pot_dirty=true) then we display
	// the current actual out pot value along with the 'dirty' dot.
	// If we're in omni mode, ignore this case.
	if (Hardware::isOutputChannelChanged && (!isOmni || in == MIDI_CHANNEL_OMNI))
	// if (is_out_pot_dirty && (!isOmni || in_pot_channel == 0))
	{
		LCD::updateOutputChannel(out, true);
	}
	// Otherwise, show what the current mapping is for the given in pot channel.
	else
	{
		LCD::updateOutputChannel(ChannelMap::getOutputChannelOf(in), false);
	}

	MIDIHandler::resetActiveState();

	if (Hardware::isButtonPressed())
	{
		commitAndSaveChannels();
	}
}

void commitAndSaveChannels()
{
	if (!Hardware::isOutputChannelChanged)
	{
		return;
	}

	// channel_map[in_pot_channel] = out_pot_channel;
	// ChannelMap::setOutputChannel(in_pot_channel, out_pot_channel);
	// EEPROM.write(CHANNEL_MAP_ADDR + in_pot_channel, out_pot_channel);
	Storage::writeChannelMapping(Hardware::inputChannel, Hardware::outputChannel);

	// is_out_pot_dirty = false;
	Hardware::resetOutputChannelChanged();
}
