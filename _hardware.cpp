// Potentiometer inputs
#define MAX_POT_VALUE 1024
#define TOTAL_POT_STEPS 17
#define CHAN_IN_POT 2
#define CHAN_OUT_POT 3
#define BUTTON_PIN 4

class Hardware
{
private:
	static float EMA_a = 0.6; //initialization of EMA alpha
	static int EMA_S_in = 0;  //initialization of EMA S for input channel
	static int EMA_S_out = 0; //initialization of EMA S for output channel
	static byte lastInputChannel = 0;
	static byte lastOutputChannel = 0;

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

public:
	static int buttonState;
	static byte inputChannel = 0;
	static byte outputChannel = 0;
	static byte isOuputChannelChanged = false;

	static void init()
	{
		pinMode(BUTTON_PIN, INPUT_PULLUP);
		readButton();
	}

	static void read()
	{
		readButton();
		readPots();
	}

	static void readButton()
	{
		buttonState = digitalRead(BUTTON_PIN);
	}

	static void readPots()
	{

		// In pot should read OMNI (0), then 1 to 16
		inputChannel = normalizePotInput(emaSmooth(analogRead(CHAN_IN_POT), &EMA_S_in), true);
		// Out pot should read OFF, then 1 to 16
		outputChannel = normalizePotInput(emaSmooth(analogRead(CHAN_OUT_POT), &EMA_S_out), false);

		// If the in pot has changed then reset the out pot dirty state
		// (in other words, any un-saved out pot value is discarded)
		if (isOuputChannelChanged && inputChannel != lastInputChannel)
		{
			isOuputChannelChanged = false;
		}
		// If the out pot has changed we set the dirty flag, meaning the
		// user has rotated the pot and is wanting to set the out channel
		else if (!isOuputChannelChanged && outputChannel != lastOutputChannel)
		{
			isOuputChannelChanged = true;
		}
		// If the out pot has changed back to the currently set value
		// of the input, the out pot is not dirty anymore
		// else if (isOuputChannelChanged && outputChannel == channel_map[inputChannel])
		// {
		// 	isOuputChannelChanged = false;
		// }

		lastInputChannel = inputChannel;
		lastOutputChannel = outputChannel;
	}

	static bool isButtonPressed()
	{
		return buttonState == LOW;
	}

	static void resetOutputChannelChanged()
	{
		isOutputChannelChanged = false;
	}
};
