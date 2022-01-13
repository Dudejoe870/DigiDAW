#pragma once

#include "audio/common.h"

namespace DigiDAW::Audio
{
	class Engine
	{
	private:
		std::unique_ptr<RtAudio> audioBackend;

		std::vector<RtAudio::Api> supportedAPIs;

		unsigned int currentOutputDevice;
		unsigned int currentInputDevice;
	public:
		struct AudioDevice
		{
			RtAudio::DeviceInfo info;
			RtAudio::Api backend;
			unsigned int index;

			AudioDevice(RtAudio::DeviceInfo info, RtAudio::Api backend, unsigned int index)
			{
				this->info = info;
				this->backend = backend;
				this->index = index;
			}
		};

		Engine(RtAudio::Api api);
		~Engine();

		ReturnCode getSupportedAPIs(std::vector<RtAudio::Api>& dest);
		ReturnCode getCurrentAPI(RtAudio::Api& api);

		ReturnCode changeBackend(RtAudio::Api api);

		ReturnCode getDevices(std::vector<AudioDevice>& dest);

		ReturnCode getAPIDisplayName(RtAudio::Api api, std::string& dest);
		ReturnCode getAPIName(RtAudio::Api api, std::string& dest);

		ReturnCode setCurrentOutputDevice(unsigned int device);
		ReturnCode setCurrentInputDevice(unsigned int device);

		ReturnCode getCurrentOutputDevice(unsigned int& device);
		ReturnCode getCurrentInputDevice(unsigned int& device);

		ReturnCode stopEngine();
	};
}
