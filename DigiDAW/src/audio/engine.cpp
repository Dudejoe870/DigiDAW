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

	ReturnCode Engine::getSupportedAPIs(std::vector<RtAudio::Api>& dest)
	{
		dest = supportedAPIs;
		return ReturnCode::Success;
	}

	ReturnCode Engine::getCurrentAPI(RtAudio::Api& api)
	{
		api = audioBackend->getCurrentApi();
		return ReturnCode::Success;
	}

	void Engine::updateDevices()
	{
		currentDevices.clear();
		for (unsigned int i = 0; i < audioBackend->getDeviceCount(); ++i)
			currentDevices.push_back(getAudioDevice(i));
	}

	ReturnCode Engine::getDevices(std::vector<Engine::AudioDevice>& dest)
	{
		dest = currentDevices;
		return ReturnCode::Success;
	}

	ReturnCode Engine::getAPIDisplayName(RtAudio::Api api, std::string& dest)
	{
		dest = RtAudio::getApiDisplayName(api);
		return ReturnCode::Success;
	}

	ReturnCode Engine::getAPIName(RtAudio::Api api, std::string& dest)
	{
		dest = RtAudio::getApiName(api);
		return ReturnCode::Success;
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

		// TODO: Reopen the stream with the new output device if we already had one open before.
		return ReturnCode::Success;
	}

	ReturnCode Engine::setCurrentInputDevice(unsigned int device)
	{
		stopEngine();

		if (device != -1 && !currentDevices[device].info.probed)
			return ReturnCode::Error;

		currentInputDevice = device;

		resetSampleRate();

		// TODO: Reopen the stream with the new input device if we already had one open before.
		return ReturnCode::Success;
	}

	ReturnCode Engine::getCurrentOutputDevice(unsigned int& device)
	{
		device = currentOutputDevice;
		return ReturnCode::Success;
	}

	ReturnCode Engine::getCurrentInputDevice(unsigned int& device)
	{
		device = currentInputDevice;
		return ReturnCode::Success;
	}

	ReturnCode Engine::setCurrentSampleRate(unsigned int sampleRate)
	{
		stopEngine();
		currentSampleRate = sampleRate;

		// TODO: Reopen the stream with the new sample rate if we already had one open before.
		return ReturnCode::Success;
	}

	ReturnCode Engine::getCurrentSampleRate(unsigned int& sampleRate)
	{
		sampleRate = currentSampleRate;
		return ReturnCode::Success;
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

	ReturnCode Engine::stopEngine()
	{
		if (audioBackend->isStreamOpen()) audioBackend->closeStream();
		return ReturnCode::Success;
	}

	ReturnCode Engine::pauseEngine()
	{
		if (audioBackend->isStreamRunning()) audioBackend->stopStream();
		return ReturnCode::Success;
	}
}
