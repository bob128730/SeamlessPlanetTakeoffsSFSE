#include "INIReader.h"

struct config 
{
	void load() 
	{
		INIReader reader("Data\\SFSE\\plugins\\SPT_Config.ini");
		if (reader.ParseError() != 0) 
		{
			REX::WARN("Failed to read config file, using default");
			return;
		}
		this->DisableTakeOffCam = reader.GetBoolean("Config", "DisableTakeOffCam", true);
	}

	bool DisableTakeOffCam = 1;
	bool GravLanesSupport = 0;
};