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
		this->EnableLoadBlur = reader.GetBoolean("Config", "EnableLoadBlur", false);
		this->EnableCameraShake = reader.GetBoolean("Config", "EnableCameraShake", true);
		this->TakeoffExtensionLength = reader.GetFloat("Config", "TakeoffExtensionLength", 7.6f);
		if (this->TakeoffExtensionLength < 4)
			this->TakeoffExtensionLength = 4.0f;
	}

	bool DisableTakeOffCam = 1;
	bool EnableLoadBlur = false;
	bool EnableCameraShake = true;
	float TakeoffExtensionLength = 7.6f;
};