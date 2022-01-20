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

		/* 
			Note: We may want to support multiple input and output devices in the future.
			However, this would require multiple instances of RtAudio to open multiple streams.
			Which I believe is a valid use of the API, but currently I'm not concerning myself with it.
		*/
		unsigned int currentOutputDevice; // -1 is reserved for the "None" (no device) option.
		unsigned int currentInputDevice;

		unsigned int currentSampleRate;
		unsigned int currentBufferSize;

		void resetSampleRate();
		AudioDevice getAudioDevice(unsigned int index);
		void updateDevices();

		unsigned int getFirstAvailableInputDevice();
		unsigned int getFirstAvailableOutputDevice();

		void initializeDevices();

		static int audioCallback(
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

		std::vector<RtAudio::Api> getSupportedAPIs();
		RtAudio::Api getCurrentAPI();

		ReturnCode changeBackend(RtAudio::Api api);

		std::vector<AudioDevice> getDevices();

		std::string getAPIDisplayName(RtAudio::Api api);
		std::string getAPIName(RtAudio::Api api);

		ReturnCode setCurrentOutputDevice(unsigned int device);
		ReturnCode setCurrentInputDevice(unsigned int device);

		unsigned int getCurrentOutputDevice();
		unsigned int getCurrentInputDevice();

		ReturnCode setCurrentSampleRate(unsigned int sampleRate);
		unsigned int getCurrentSampleRate();

		ReturnCode setCurrentBufferSize(unsigned int bufferSize);
		unsigned int getCurrentBufferSize();

		ReturnCode getSupportedSampleRates(std::vector<unsigned int>& sampleRates);

		ReturnCode openStream();
		ReturnCode startEngine();

		ReturnCode stopEngine();

		bool isStreamOpen();
		bool isStreamRunning();
	};
}
