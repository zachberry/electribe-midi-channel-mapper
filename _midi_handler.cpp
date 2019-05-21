#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

class MIDIHandler
{
private:
	// Array to keep track of which channels are active. We use this information to display
	// a dot in the LED display next to the input channel (so the user can see visually
	// which channels are getting messages)
	static bool active_map[17] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	static void setChannelIsActive(byte channel)
	{
		active_map[MIDI_CHANNEL_OMNI] = true;
		active_map[channel] = true;
	}

	static void handleProgramChange(byte channel, byte number)
	{
		setChannelIsActive(channel);
		MIDI.sendProgramChange(number, getOutChannel(channel));
	}

	static void handleNoteOn(byte channel, byte note, byte velocity)
	{
		setChannelIsActive(channel);
		MIDI.sendNoteOn(note, velocity, getOutChannel(channel));
	}

	static void handleNoteOff(byte channel, byte note, byte velocity)
	{
		setChannelIsActive(channel);
		MIDI.sendNoteOff(note, velocity, getOutChannel(channel));
	}

	static void handlePitchBend(byte channel, int bend)
	{
		setChannelIsActive(channel);
		MIDI.sendPitchBend(bend, getOutChannel(channel));
	}

	static void handleControlChange(byte channel, byte number, byte value)
	{
		setChannelIsActive(channel);
		MIDI.sendControlChange(number, value, getOutChannel(channel));
	}

	static void handleAfterTouchPolyPressure(byte channel, byte note, byte pressure)
	{
		setChannelIsActive(channel);
		MIDI.sendAfterTouch(note, pressure, getOutChannel(channel));
	}

	static void handleAfterTouchChannelPressure(byte channel, byte pressure)
	{
		setChannelIsActive(channel);
		MIDI.sendAfterTouch(pressure, getOutChannel(channel));
	}

	static void handleTimeCodeQuarterFrame(byte data)
	{
		MIDI.sendTimeCodeQuarterFrame(data);
	}

	static void handleSongPosition(unsigned beats)
	{
		MIDI.sendSongPosition(beats);
	}

	static void handleSongSelect(byte songnumber)
	{
		MIDI.sendSongSelect(songnumber);
	}

	static void handleTuneRequest()
	{
		MIDI.sendTuneRequest();
	}

	// Real time messages (Messages that don't specify channels)
	// (These messages don't have their own sendX method, instead we must use a
	// lower-level send method from the library):
	static void handleClock()
	{
		MIDI.sendRealTime(midi::Clock);
	}

	static void handleStart()
	{
		MIDI.sendRealTime(midi::Start);
	}

	static void handleContinue()
	{
		MIDI.sendRealTime(midi::Continue);
	}

	static void handleStop()
	{
		MIDI.sendRealTime(midi::Stop);
	}

	static void handleActiveSensing()
	{
		MIDI.sendRealTime(midi::ActiveSensing);
	}

	static void handleSystemReset()
	{
		MIDI.sendRealTime(midi::SystemReset);
	}

public:
	static void init()
	{
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
	}

	static bool isChannelActive(byte channel)
	{
		return active_map[channel];
	}

	static void resetActiveState()
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
}
