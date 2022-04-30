#pragma once

#include "audio/common.h"

#include "audio/trackstate.h"
#include "audio/mixer.h"

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

		std::vector<unsigned int> currentSupportedSampleRates;

		/* 
			Note: We may want to support multiple input and output devices in the future.
			However, this would require multiple instances of RtAudio to open multiple streams.
			Which I believe is a valid use of the API, but currently I'm not concerning myself with it.
		*/
		unsigned int currentOutputDevice; // -1 is reserved for the "None" (no device) option.
		unsigned int currentInputDevice;

		unsigned int currentSampleRate;
		unsigned int currentBufferSize;

		void UpdateCurrentSupportedSampleRates();
		void ResetSampleRate();
		AudioDevice GetAudioDevice(unsigned int index);
		void UpdateDevices();

		unsigned int GetFirstAvailableInputDevice();
		unsigned int GetFirstAvailableOutputDevice();

		void InitializeDevices();

		static int AudioCallback(
			void* outputBuffer, 
			void* inputBuffer, 
			unsigned int nFrames, 
			double streamTime, 
			RtAudioStreamStatus status, 
			void* userData);
	public:
		TrackState trackState;
		Mixer mixer;

		Engine(RtAudio::Api api);
		~Engine();

		std::vector<RtAudio::Api> GetSupportedAPIs();
		RtAudio::Api GetCurrentAPI();

		ReturnCode ChangeBackend(RtAudio::Api api);

		std::vector<AudioDevice> GetDevices();

		std::string GetAPIDisplayName(RtAudio::Api api);
		std::string GetAPIName(RtAudio::Api api);

		ReturnCode SetCurrentOutputDevice(unsigned int device);
		ReturnCode SetCurrentInputDevice(unsigned int device);

		unsigned int GetCurrentOutputDevice();
		unsigned int GetCurrentInputDevice();

		ReturnCode SetCurrentSampleRate(unsigned int sampleRate);
		unsigned int GetCurrentSampleRate();

		ReturnCode SetCurrentBufferSize(unsigned int bufferSize);
		unsigned int GetCurrentBufferSize();

		ReturnCode GetSupportedSampleRates(std::vector<unsigned int>& sampleRates);
		ReturnCode GetSupportedSampleRates(std::vector<unsigned int>& sampleRates, unsigned int outputDevice, unsigned int inputDevice);
		ReturnCode GetSupportedSampleRates(std::vector<unsigned int>& sampleRates, AudioDevice outputDevice, AudioDevice inputDevice);

		std::vector<unsigned int> GetCurrentSupportedSampleRates();

		ReturnCode OpenStream();
		ReturnCode StartEngine();

		ReturnCode StopEngine();

		bool IsStreamOpen();
		bool IsStreamRunning();
	};
}
