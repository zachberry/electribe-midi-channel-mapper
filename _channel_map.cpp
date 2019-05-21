class ChannelMap
{
private:
	// Array to hold inputs and outputs.  Index is input channel, value is output channel.
	// 0 = No output, 1 = Midi Channel 1, ..., 16 = Midi Channel 16
	static byte channel_map[17] = {MIDI_CHANNEL_OFF, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

	static void writeChannel(int side, byte channelNum, bool dot)
	{
	}

public:
	static byte inputChannel = 0;
	static byte outputChannel = 0;

	static void init()
	{
	}

	static bool isOMNIMode()
	{
		return channel_map[MIDI_CHANNEL_OMNI] != MIDI_CHANNEL_OFF;
	}

	static bool getOutputChannel()
	{
		if (isOMNIMode())
		{
			return channel_map[MIDI_CHANNEL_OMNI];
		}

		return channel_map[inputChannel];
	}

	static void setOuputChannel(byte inputChannel, byte outputChannel)
	{
		channel_map[inputChannel] = outputChannel;
	}

	static void disableAllChannels()
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
	}
};
