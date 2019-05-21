#include <EEPROM.h>

// EEPROM Addresses
#define ADDR_VERSION 0	 // value of 1 if memory has been initialized
#define ADDR_CHANNEL_MAP 1 // 1 - 17

class Storage
{
private:
	byte _softwareVersion = 1;

	void getVersionNum()
	{
		return EEPROM.get(ADDR_VERSION);
	}

public:
	static byte[] init(softwareVersion, defaultChannelMap, forceReset = false)
	{
		byte _softwareVersion = softwareVersion;
		bool isEEPROMValid = getVersionNum() == _softwareVersion;

		if (forceReset || !isEEPROMValid)
		{
			writeChannelMap(defaultChannelMap);
		}

		return getChannelMap();
	}

	static void writeChannelMap(channelMap)
	{
		EEPROM.write(ADDR_VERSION, _softwareVersion);
		EEPROM.put(ADDR_CHANNEL_MAP, channelMap);
	}

	static void getChannelMap()
	{
		byte[] = channelMap;
		EEPROM.get(ADDR_CHANNEL_MAP, channelMap);

		return channelMap;
	}
};
