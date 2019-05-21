#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

// Matrix raw display values
#define MATRIX_BLANK 0B000000000
#define MATRIX_DASH 0B001000000

#define DIGIT_1 0
#define DIGIT_2 1
#define DIGIT_3 3
#define DIGIT_4 4

#define LEFT 0
#define RIGHT 1

class LCD
{
private:
	static void writeChannel(int side, byte channelNum, bool dot)
	{
		int leftDigit = channelNum / 10;
		int rightDigit = channelNum % 10;

		switch (side)
		{
		case LEFT:
			int firstDigit = DIGIT_1;
			int secondDigit = DIGIT_2;
			break;

		case RIGHT:
			int firstDigit = DIGIT_3;
			int secondDigit = DIGIT_4;
		}

		if (leftDigit == 0)
		{
			matrix.writeDigitRaw(firstDigit, MATRIX_BLANK, false);
		}
		else
		{
			matrix.writeDigitNum(firstDigit, leftDigit, false);
		}

		matrix.writeDigitNum(secondDigit, rightDigit, dot);

		matrix.writeDisplay();
	}

public:
	static void init()
	{
		// Init display
		matrix.begin(0x70);
		matrix.setBrightness(1);
	}

	static void writeRaw(digit1, digit2, digit3, digit4, colon = false, dot1 = false, dot2 = false, dot3 = false, dot4 = false)
	{
		matrix.drawColon(colon);
		matrix.writeDigitRaw(DIGIT_1, digit1, dot1);
		matrix.writeDigitRaw(DIGIT_2, digit2, dot2);
		matrix.writeDigitRaw(DIGIT_3, digit3, dot3);
		matrix.writeDigitRaw(DIGIT_4, digit4, dot4);
		matrix.writeDisplay();
	}

	static void writeNum(digit1, digit2, digit3, digit4, colon = false, dot1 = false, dot2 = false, dot3 = false, dot4 = false)
	{
		matrix.drawColon(colon);
		matrix.writeDigitNum(DIGIT_1, digit1, dot1);
		matrix.writeDigitNum(DIGIT_2, digit2, dot2);
		matrix.writeDigitNum(DIGIT_3, digit3, dot3);
		matrix.writeDigitNum(DIGIT_4, digit4, dot4);
		matrix.writeDisplay();
	}

	static void clear()
	{
		writeRaw(MATRIX_BLANK, MATRIX_BLANK, MATRIX_BLANK, MATRIX_BLANK);
	}

	static void displayAllDashes()
	{
		writeRaw(MATRIX_DASH, MATRIX_DASH, MATRIX_DASH, MATRIX_DASH);
	}

	static void displayAllZeroes()
	{
		writeNum(0, 0, 0, 0);
	}

	static void writeInputChannel(byte channelNum, bool dot)
	{
		writeChannel(LEFT, channelNum, dot);
	}

	// NOTE: MIDI_CHANNEL_OFF is actually 17, but we want to display that
	// on the LCD as "--".
	static void writeOuputChannel(byte channelNum, bool dot)
	{
		if (val == MIDI_CHANNEL_OFF)
		{
			// Display "--"
			matrix.writeDigitRaw(DIGIT_3, MATRIX_DASH, false);
			matrix.writeDigitRaw(DIGIT_4, MATRIX_DASH, false);
			matrix.writeDisplay();
			return;
		}

		writeChannel(RIGHT, channelNum, dot);
	}

	static void writeColon(bool colon)
	{
		matrix.drawColon(colon);
	}
}
