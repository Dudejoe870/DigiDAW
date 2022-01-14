#pragma once

#include "audio/common.h"

namespace DigiDAW::Audio
{
	class Engine
	{
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

			AudioDevice()
			{
				this->info = RtAudio::DeviceInfo();
				this->backend = RtAudio::Api::UNSPECIFIED;
				this->index = -1;
			}
		};
	private:
		std::unique_ptr<RtAudio> audioBackend;

		std::vector<RtAudio::Api> supportedAPIs;

		std::vector<AudioDevice> currentDevices;

		/* 
			Note: We may want to support multiple input and output devices in the future.
			However, this would require multiple instances of RtAudio to open multiple streams.
			Which I believe is a valid use of the API, but currently I'm not concerning myself with it.
		*/
		unsigned int currentOutputDevice; // -1 is reserved for the "None" (no device) option.
		unsigned int currentInputDevice;

		unsigned int currentSampleRate;

		void resetSampleRate();
		AudioDevice getAudioDevice(unsigned int index);
		void updateDevices();

		unsigned int getFirstAvailableInputDevice();
		unsigned int getFirstAvailableOutputDevice();

		void initializeDevices();
	public:
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

		ReturnCode setCurrentSampleRate(unsigned int sampleRate);
		ReturnCode getCurrentSampleRate(unsigned int& sampleRate);

		ReturnCode getSupportedSampleRates(std::vector<unsigned int>& sampleRates);

		ReturnCode stopEngine();
		ReturnCode pauseEngine();
	};
}
