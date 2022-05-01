#include "audio/engine.h"

namespace DigiDAW::Audio
{
	Engine::Engine(RtAudio::Api api)
		: mixer(*this)
	{
		audioBackend = std::make_unique<RtAudio>(api);

		std::vector<RtAudio::Api> compiledAPIs;
		RtAudio::getCompiledApi(compiledAPIs);

		for (RtAudio::Api api : compiledAPIs)
		{
			RtAudio tempAudio(api);
			if (tempAudio.getDeviceCount() > 0) supportedAPIs.push_back(api);
		}

		InitializeDevices();
	}

	Engine::~Engine()
	{
		if (audioBackend->isStreamOpen()) audioBackend->closeStream();
	}

	void Engine::InitializeDevices()
	{
		UpdateDevices();

		currentOutputDevice = GetFirstAvailableOutputDevice();
		if (currentInputDevice != -1) currentInputDevice = GetFirstAvailableInputDevice();

		currentBufferSize = 512;

		UpdateCurrentSupportedSampleRates();
		ResetSampleRate(); // Also opens stream
	}

	unsigned int Engine::GetFirstAvailableInputDevice()
	{
		const unsigned int def = audioBackend->getDefaultInputDevice();
		if (currentDevices[def].info.probed) return def;
		for (Engine::AudioDevice device : currentDevices)
			if (device.info.probed && device.info.inputChannels > 0) return device.index;

		return -1;
	}

	unsigned int Engine::GetFirstAvailableOutputDevice()
	{
		const unsigned int def = audioBackend->getDefaultOutputDevice();
		if (currentDevices[def].info.probed) return def;
		for (Engine::AudioDevice device : currentDevices)
			if (device.info.probed && device.info.outputChannels > 0) return device.index;

		return -1;
	}

	std::vector<RtAudio::Api>& Engine::GetSupportedAPIs()
	{
		return supportedAPIs;
	}

	RtAudio::Api Engine::GetCurrentAPI()
	{
		return audioBackend->getCurrentApi();
	}

	void Engine::UpdateDevices()
	{
		currentDevices.clear();
		for (unsigned int i = 0; i < audioBackend->getDeviceCount(); ++i)
			currentDevices.push_back(GetAudioDevice(i));
	}

	Engine::AudioDevice Engine::GetAudioDevice(unsigned int index)
	{
		return Engine::AudioDevice(audioBackend->getDeviceInfo(index), audioBackend->getCurrentApi(), index);
	}

	std::vector<Engine::AudioDevice>& Engine::GetDevices()
	{
		return currentDevices;
	}

	std::string Engine::GetAPIDisplayName(RtAudio::Api api)
	{
		return RtAudio::getApiDisplayName(api);
	}

	std::string Engine::GetAPIName(RtAudio::Api api)
	{
		return RtAudio::getApiName(api);
	}

	void Engine::UpdateCurrentSupportedSampleRates()
	{
		GetSupportedSampleRates(currentSupportedSampleRates, currentOutputDevice, currentInputDevice);
	}

	void Engine::ResetSampleRate()
	{
		if (currentOutputDevice == -1 && currentInputDevice != -1)
		{
			if (!currentDevices[currentInputDevice].info.probed)
			{
				currentSampleRate = 0;
				return;
			}
			std::vector<unsigned int>& sampleRates = currentDevices[currentInputDevice].info.sampleRates;
			SetCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (currentInputDevice == -1 && currentOutputDevice != -1)
		{
			if (!currentDevices[currentOutputDevice].info.probed)
			{
				currentSampleRate = 0;
				return;
			}
			std::vector<unsigned int>& sampleRates = currentDevices[currentOutputDevice].info.sampleRates;
			SetCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (currentInputDevice == -1 && currentOutputDevice == -1)
		{
			currentSampleRate = 0;
			return;
		}

		unsigned int bestSampleRate = 0;

		RtAudio::DeviceInfo outputInfo = currentDevices[currentOutputDevice].info;
		RtAudio::DeviceInfo inputInfo = currentDevices[currentInputDevice].info;

		if (!outputInfo.probed && inputInfo.probed)
		{
			std::vector<unsigned int>& sampleRates = currentDevices[currentInputDevice].info.sampleRates;
			SetCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (!inputInfo.probed && outputInfo.probed)
		{
			std::vector<unsigned int>& sampleRates = currentDevices[currentOutputDevice].info.sampleRates;
			SetCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (!inputInfo.probed && !outputInfo.probed)
		{
			currentSampleRate = 0;
			return;
		}

		for (unsigned int rate1 : outputInfo.sampleRates)
		{
			for (unsigned int rate2 : inputInfo.sampleRates)
			{
				if (rate1 == rate2)
				{
					if (rate1 > bestSampleRate) bestSampleRate = rate1;
					break;
				}
			}
		}

		SetCurrentSampleRate(bestSampleRate);
	}

	ReturnCode Engine::SetCurrentOutputDevice(unsigned int device)
	{
		if (currentOutputDevice == device)
			return ReturnCode::Success;

		if (device != -1 && (!currentDevices[device].info.probed || currentDevices[device].info.outputChannels == 0))
			return ReturnCode::Error;

		currentOutputDevice = device;

		ResetSampleRate();
		UpdateCurrentSupportedSampleRates();

		OpenStream();
		return ReturnCode::Success;
	}

	ReturnCode Engine::SetCurrentInputDevice(unsigned int device)
	{
		if (currentInputDevice == device)
			return ReturnCode::Success;

		if (device != -1 && (!currentDevices[device].info.probed || currentDevices[device].info.inputChannels == 0))
			return ReturnCode::Error;

		currentInputDevice = device;

		ResetSampleRate();
		UpdateCurrentSupportedSampleRates();

		OpenStream();
		return ReturnCode::Success;
	}

	unsigned int Engine::GetCurrentOutputDevice()
	{
		return currentOutputDevice;
	}

	unsigned int Engine::GetCurrentInputDevice()
	{
		return currentInputDevice;
	}

	ReturnCode Engine::SetCurrentSampleRate(unsigned int sampleRate)
	{
		currentSampleRate = sampleRate;

		OpenStream();
		return ReturnCode::Success;
	}

	unsigned int Engine::GetCurrentSampleRate()
	{
		return currentSampleRate;
	}

	ReturnCode Engine::SetCurrentBufferSize(unsigned int bufferSize)
	{
		currentBufferSize = bufferSize;

		OpenStream();
		return ReturnCode::Success;
	}

	unsigned int Engine::GetCurrentBufferSize()
	{
		return currentBufferSize;
	}

	std::vector<unsigned int>& Engine::GetSupportedSampleRates()
	{
		return currentSupportedSampleRates;
	}

	ReturnCode Engine::GetSupportedSampleRates(std::vector<unsigned int>& sampleRates)
	{
		sampleRates = currentSupportedSampleRates;
		return ReturnCode::Success;
	}

	ReturnCode Engine::GetSupportedSampleRates(std::vector<unsigned int>& sampleRates, unsigned int outputDevice, unsigned int inputDevice)
	{
		// Sanitize the devices and make sure they aren't -1 (aka None), if they are return the other one.
		if (outputDevice == -1 && inputDevice != -1)
		{
			sampleRates = currentDevices[inputDevice].info.sampleRates;
			return ReturnCode::Success;
		}
		else if (inputDevice == -1 && outputDevice != -1)
		{
			sampleRates = currentDevices[outputDevice].info.sampleRates;
			return ReturnCode::Success;
		}
		else if (inputDevice == -1 && outputDevice == -1) // If we don't have any devices selected, return error.
		{
			sampleRates.clear();
			return ReturnCode::Error;
		}

		return GetSupportedSampleRates(sampleRates, currentDevices[outputDevice], currentDevices[inputDevice]);
	}

	ReturnCode Engine::GetSupportedSampleRates(std::vector<unsigned int>& sampleRates, Engine::AudioDevice outputDevice, Engine::AudioDevice inputDevice)
	{
		RtAudio::DeviceInfo outputInfo = outputDevice.info;
		RtAudio::DeviceInfo inputInfo = inputDevice.info;

		// Make sure we're able to probe the devices, if not return the one we are able to probe.
		if (!outputInfo.probed && inputInfo.probed)
		{
			sampleRates = inputInfo.sampleRates;
			return ReturnCode::Success;
		}
		else if (!inputInfo.probed && outputInfo.probed)
		{
			sampleRates = outputInfo.sampleRates;
			return ReturnCode::Success;
		}
		else if (!inputInfo.probed && !outputInfo.probed) // If we can't probe either, return error.
		{
			sampleRates.clear();
			return ReturnCode::Error;
		}

		sampleRates.clear();

		// Get the common sample rates between the two devices.
		std::vector<unsigned int> outRates = outputInfo.sampleRates;
		std::vector<unsigned int> inRates = inputInfo.sampleRates;
		std::sort(outRates.begin(), outRates.end());
		std::sort(inRates.begin(), inRates.end());

		std::set_intersection(
			outRates.begin(), outRates.end(),
			inRates.begin(), inRates.end(),
			std::back_inserter(sampleRates));

		return ReturnCode::Success;
	}

	ReturnCode Engine::ChangeBackend(RtAudio::Api api)
	{
		audioBackend->closeStream();
		delete audioBackend.release();

		audioBackend = std::make_unique<RtAudio>(api);

		InitializeDevices();

		return ReturnCode::Success;
	}

	int Engine::AudioCallback(
		void* outputBuffer,
		void* inputBuffer,
		unsigned int nFrames,
		double streamTime,
		RtAudioStreamStatus status,
		void* userData)
	{
		Engine* engine = (Engine*)userData;

		if (!outputBuffer || engine->currentOutputDevice == -1) return 2; // Abort stream

		float* outBuf = (float*)outputBuffer;
		float* inBuf = (float*)inputBuffer;

		unsigned int nOutChannels = engine->currentDevices[engine->currentOutputDevice].info.outputChannels;
		unsigned int nInChannels = (engine->currentInputDevice != -1) ? engine->currentDevices[engine->currentInputDevice].info.inputChannels : 0;

		std::memset(outBuf, 0, sizeof(float) * (nOutChannels * nFrames));
		engine->mixer.Mix(outBuf, inBuf, streamTime, nFrames, nOutChannels, nInChannels, engine->currentSampleRate);

		return 0;
	}

	ReturnCode Engine::OpenStream()
	{
		if (currentOutputDevice == -1)
			return ReturnCode::Error;

		if (IsStreamOpen()) audioBackend->closeStream();

		RtAudio::StreamParameters outputParams;
		outputParams.deviceId = currentOutputDevice;
		outputParams.firstChannel = 0;
		outputParams.nChannels = currentDevices[currentOutputDevice].info.outputChannels;

		RtAudio::StreamParameters inputParams;
		if (currentInputDevice != -1)
		{
			inputParams.deviceId = currentInputDevice;
			inputParams.firstChannel = 0;
			inputParams.nChannels = currentDevices[currentInputDevice].info.inputChannels;
		}

		RtAudio::StreamOptions options;
		options.flags = RTAUDIO_NONINTERLEAVED;
		options.numberOfBuffers = 0;
		options.priority = 0;
		options.streamName = "main";

		unsigned int bufferSize = currentBufferSize;
		RtAudioErrorType streamError =
			audioBackend->openStream(
				&outputParams,
				(currentInputDevice != -1 ? &inputParams : NULL),
				RTAUDIO_FLOAT32,
				currentSampleRate,
				&bufferSize,
				AudioCallback, this, &options);

		if (streamError != RTAUDIO_NO_ERROR)
			return ReturnCode::Error;

		currentBufferSize = bufferSize;
		mixer.UpdateTrackBuffers();
		mixer.UpdateBusBuffers();

		return ReturnCode::Success;
	}

	ReturnCode Engine::StartEngine()
	{
		if (!IsStreamRunning())
		{
			if (!IsStreamOpen())
			{
				ReturnCode streamError = OpenStream();
				if (streamError != ReturnCode::Success)
					return streamError;
			}

			audioBackend->startStream();
			mixer.UpdateCurrentTime(audioBackend->getStreamTime());
		}

		return ReturnCode::Success;
	}

	ReturnCode Engine::StopEngine()
	{
		if (IsStreamRunning()) audioBackend->stopStream();
		return ReturnCode::Success;
	}

	bool Engine::IsStreamOpen()
	{
		return audioBackend->isStreamOpen();
	}

	bool Engine::IsStreamRunning()
	{
		return audioBackend->isStreamRunning();
	}
}
