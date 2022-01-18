#include "audio/engine.h"

namespace DigiDAW::Audio
{
	Engine::Engine(RtAudio::Api api)
	{
		audioBackend = std::make_unique<RtAudio>(api);

		std::vector<RtAudio::Api> compiledAPIs;
		RtAudio::getCompiledApi(compiledAPIs);

		for (RtAudio::Api api : compiledAPIs)
		{
			RtAudio testAudio(api);
			if (testAudio.getDeviceCount() > 0) supportedAPIs.push_back(api);
		}

		initializeDevices();
	}

	Engine::~Engine()
	{
		stopEngine();
	}

	void Engine::initializeDevices()
	{
		updateDevices();

		currentOutputDevice = getFirstAvailableOutputDevice();
		if (currentInputDevice != -1) currentInputDevice = getFirstAvailableInputDevice();

		currentBufferSize = 512;

		resetSampleRate();
	}

	unsigned int Engine::getFirstAvailableInputDevice()
	{
		const unsigned int def = audioBackend->getDefaultInputDevice();
		if (currentDevices[def].info.probed) return def;
		for (Engine::AudioDevice device : currentDevices)
			if (device.info.probed && device.info.inputChannels > 0) return device.index;

		return -1;
	}

	unsigned int Engine::getFirstAvailableOutputDevice()
	{
		const unsigned int def = audioBackend->getDefaultOutputDevice();
		if (currentDevices[def].info.probed) return def;
		for (Engine::AudioDevice device : currentDevices)
			if (device.info.probed && device.info.outputChannels > 0) return device.index;

		return -1;
	}

	std::vector<RtAudio::Api> Engine::getSupportedAPIs()
	{
		return supportedAPIs;
	}

	RtAudio::Api Engine::getCurrentAPI()
	{
		return audioBackend->getCurrentApi();
	}

	void Engine::updateDevices()
	{
		currentDevices.clear();
		for (unsigned int i = 0; i < audioBackend->getDeviceCount(); ++i)
			currentDevices.push_back(getAudioDevice(i));
	}

	std::vector<Engine::AudioDevice> Engine::getDevices()
	{
		return currentDevices;
	}

	std::string Engine::getAPIDisplayName(RtAudio::Api api)
	{
		return RtAudio::getApiDisplayName(api);
	}

	std::string Engine::getAPIName(RtAudio::Api api)
	{
		return RtAudio::getApiName(api);
	}

	Engine::AudioDevice Engine::getAudioDevice(unsigned int index)
	{
		return Engine::AudioDevice(audioBackend->getDeviceInfo(index), audioBackend->getCurrentApi(), index);
	}

	void Engine::resetSampleRate()
	{
		if (currentOutputDevice == -1 && currentInputDevice != -1)
		{
			if (!currentDevices[currentInputDevice].info.probed)
			{
				currentSampleRate = 0;
				return;
			}
			std::vector<unsigned int>& sampleRates = currentDevices[currentInputDevice].info.sampleRates;
			setCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
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
			setCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
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
			setCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (!inputInfo.probed && outputInfo.probed)
		{
			std::vector<unsigned int>& sampleRates = currentDevices[currentOutputDevice].info.sampleRates;
			setCurrentSampleRate(*std::max_element(sampleRates.begin(), sampleRates.end()));
			return;
		}
		else if (!inputInfo.probed && !outputInfo.probed)
		{
			currentSampleRate = 0;
			return;
		}

		// As an optimization, use the smaller of the two sample rate lists for the outer loop to minimize iterations.
		std::vector<unsigned int>& rates1 =
			outputInfo.sampleRates.size() < inputInfo.sampleRates.size() ? outputInfo.sampleRates : inputInfo.sampleRates;
		std::vector<unsigned int>& rates2 =
			rates1 == outputInfo.sampleRates ? inputInfo.sampleRates : outputInfo.sampleRates;

		for (unsigned int rate1 : rates1)
		{
			for (unsigned int rate2 : rates2)
			{
				if (rate1 == rate2)
				{
					if (rate1 > bestSampleRate) bestSampleRate = rate1;
					break;
				}
			}
		}

		setCurrentSampleRate(bestSampleRate);
	}

	ReturnCode Engine::setCurrentOutputDevice(unsigned int device)
	{
		stopEngine();

		if (device != -1 && !currentDevices[device].info.probed)
			return ReturnCode::Error;

		currentOutputDevice = device;

		resetSampleRate();
		return ReturnCode::Success;
	}

	ReturnCode Engine::setCurrentInputDevice(unsigned int device)
	{
		stopEngine();

		if (device != -1 && !currentDevices[device].info.probed)
			return ReturnCode::Error;

		currentInputDevice = device;

		resetSampleRate();
		return ReturnCode::Success;
	}

	unsigned int Engine::getCurrentOutputDevice()
	{
		return currentOutputDevice;
	}

	unsigned int Engine::getCurrentInputDevice()
	{
		return currentInputDevice;
	}

	ReturnCode Engine::setCurrentSampleRate(unsigned int sampleRate)
	{
		stopEngine();
		currentSampleRate = sampleRate;

		return ReturnCode::Success;
	}

	unsigned int Engine::getCurrentSampleRate()
	{
		return currentSampleRate;
	}

	ReturnCode Engine::setCurrentBufferSize(unsigned int bufferSize)
	{
		stopEngine();
		currentBufferSize = bufferSize;

		return ReturnCode::Success;
	}

	unsigned int Engine::getCurrentBufferSize()
	{
		return currentBufferSize;
	}

	ReturnCode Engine::getSupportedSampleRates(std::vector<unsigned int>& sampleRates)
	{
		// Sanitize the devices and make sure they aren't -1 (aka None), if they are return the other one.
		if (currentOutputDevice == -1 && currentInputDevice != -1)
		{
			sampleRates = currentDevices[currentInputDevice].info.sampleRates;
			return ReturnCode::Success;
		}
		else if (currentInputDevice == -1 && currentOutputDevice != -1)
		{
			sampleRates = currentDevices[currentOutputDevice].info.sampleRates;
			return ReturnCode::Success;
		}
		else if (currentInputDevice == -1 && currentOutputDevice == -1) // If we don't have any devices selected, return error.
		{
			sampleRates.clear();
			return ReturnCode::Error;
		}

		RtAudio::DeviceInfo outputInfo = currentDevices[currentOutputDevice].info;
		RtAudio::DeviceInfo inputInfo = currentDevices[currentInputDevice].info;

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

	ReturnCode Engine::changeBackend(RtAudio::Api api)
	{
		stopEngine();
		delete audioBackend.release();

		audioBackend = std::make_unique<RtAudio>(api);

		initializeDevices();

		return ReturnCode::Success;
	}

	int Engine::audioCallback(
		void* outputBuffer,
		void* inputBuffer,
		unsigned int nFrames,
		double streamTime,
		RtAudioStreamStatus status,
		void* userData)
	{
		Engine* engine = (Engine*)userData;

		if (!outputBuffer || engine->currentOutputDevice == -1) return 2;

		float* outBuf = (float*)outputBuffer;
		float* inBuf = (float*)inputBuffer;

		unsigned int nOutChannels = engine->currentDevices[engine->currentOutputDevice].info.outputChannels;
		unsigned int nInChannels = (engine->currentInputDevice != -1) ? engine->currentDevices[engine->currentInputDevice].info.inputChannels : 0;

		std::memset(outBuf, 0, sizeof(float) * (nOutChannels * nFrames));
		engine->mixer.mix(outBuf, inBuf, streamTime, nFrames, nOutChannels, nInChannels, engine->currentSampleRate);

		return 0;
	}

	ReturnCode Engine::startEngine()
	{
		if (isStreamOpen() && !isStreamRunning())
		{
			audioBackend->startStream();
			mixer.updateCurrentTime(audioBackend->getStreamTime());
			return ReturnCode::Success;
		}

		if (currentOutputDevice == -1)
			return ReturnCode::Error;

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
				audioCallback, this, &options);

		audioBackend->startStream();
		mixer.updateCurrentTime(audioBackend->getStreamTime());

		if (streamError != RTAUDIO_NO_ERROR)
			return ReturnCode::Error;

		return ReturnCode::Success;
	}

	ReturnCode Engine::stopEngine()
	{
		if (isStreamOpen()) audioBackend->closeStream();
		return ReturnCode::Success;
	}

	ReturnCode Engine::pauseEngine()
	{
		if (isStreamRunning()) audioBackend->stopStream();
		return ReturnCode::Success;
	}

	bool Engine::isStreamOpen()
	{
		return audioBackend->isStreamOpen();
	}

	bool Engine::isStreamRunning()
	{
		return audioBackend->isStreamRunning();
	}
}
