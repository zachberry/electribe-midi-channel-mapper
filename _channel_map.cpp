class ChannelMap
{
private:
	// static void writeChannel(int side, byte channelNum, bool dot)
	// {
	// }

public:
	// static byte inputChannel = 0;
	// static byte outputChannel = 0;

	static byte defaultMap[17] = {MIDI_CHANNEL_OFF, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

	// Array to hold inputs and outputs.  Index is input channel, value is output channel.
	// 0 = No output, 1 = Midi Channel 1, ..., 16 = Midi Channel 16
	static byte map[17] = {MIDI_CHANNEL_OFF, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

	static void init(byte channelMap)
	{
		map = channelMap;
	}

	static bool isOMNIMode()
	{
		return map[MIDI_CHANNEL_OMNI] != MIDI_CHANNEL_OFF;
	}

	static bool getOutputChannelOf(inputChannel)
	{
		if (isOMNIMode())
		{
			return map[MIDI_CHANNEL_OMNI];
		}

		return map[inputChannel];
	}

	static void setOuputChannel(byte inputChannel, byte outputChannel)
	{
		map[inputChannel] = outputChannel;
	}

	static void disableAllChannels()
	{
		map[MIDI_CHANNEL_OMNI] = MIDI_CHANNEL_OFF;
		map[1] = MIDI_CHANNEL_OFF;
		map[2] = MIDI_CHANNEL_OFF;
		map[3] = MIDI_CHANNEL_OFF;
		map[4] = MIDI_CHANNEL_OFF;
		map[5] = MIDI_CHANNEL_OFF;
		map[6] = MIDI_CHANNEL_OFF;
		map[7] = MIDI_CHANNEL_OFF;
		map[8] = MIDI_CHANNEL_OFF;
		map[9] = MIDI_CHANNEL_OFF;
		map[10] = MIDI_CHANNEL_OFF;
		map[11] = MIDI_CHANNEL_OFF;
		map[12] = MIDI_CHANNEL_OFF;
		map[13] = MIDI_CHANNEL_OFF;
		map[14] = MIDI_CHANNEL_OFF;
		map[15] = MIDI_CHANNEL_OFF;
		map[16] = MIDI_CHANNEL_OFF;
	}
};
