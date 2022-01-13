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

		updateDevices();

		currentOutputDevice = audioBackend->getDefaultOutputDevice();
		currentInputDevice = audioBackend->getDefaultInputDevice();

		resetSampleRate();
	}

	Engine::~Engine()
	{
		stopEngine();
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
		unsigned int bestSampleRate = 0;

		RtAudio::DeviceInfo outputInfo = currentDevices[currentOutputDevice].info;
		RtAudio::DeviceInfo inputInfo = currentDevices[currentInputDevice].info;

		if (!outputInfo.probed || !inputInfo.probed)
		{
			currentSampleRate = 0;
			return;
		}

		// As an optimization, use the smaller of the two sample rate lists for the outer loop.
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

		unsigned int previousDevice = currentOutputDevice;
		currentOutputDevice = device;
		if (!currentDevices[currentOutputDevice].info.probed)
		{
			currentOutputDevice = previousDevice;
			return ReturnCode::Error;
		}

		resetSampleRate();

		// TODO: Reopen the stream with the new output device if we already had one open before.
		return ReturnCode::Success;
	}

	ReturnCode Engine::setCurrentInputDevice(unsigned int device)
	{
		stopEngine();

		unsigned int previousDevice = currentInputDevice;
		currentInputDevice = device;
		if (!currentDevices[currentInputDevice].info.probed)
		{
			currentInputDevice = previousDevice;
			return ReturnCode::Error;
		}

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
		RtAudio::DeviceInfo outputInfo = currentDevices[currentOutputDevice].info;
		RtAudio::DeviceInfo inputInfo = currentDevices[currentInputDevice].info;

		if (!outputInfo.probed || !inputInfo.probed)
		{
			sampleRates.clear();
			return ReturnCode::Error;
		}

		// As an optimization, use the smaller of the two sample rate lists for the outer loop.
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
					sampleRates.push_back(rate1);
					break;
				}
			}
		}

		return ReturnCode::Success;
	}

	ReturnCode Engine::changeBackend(RtAudio::Api api)
	{
		stopEngine();
		delete audioBackend.release();

		audioBackend = std::make_unique<RtAudio>(api);

		updateDevices();

		currentOutputDevice = audioBackend->getDefaultOutputDevice();
		currentInputDevice = audioBackend->getDefaultInputDevice();

		resetSampleRate();

		return ReturnCode::Success;
	}

	ReturnCode Engine::stopEngine()
	{
		if (audioBackend->isStreamOpen()) audioBackend->closeStream();
		return ReturnCode::Success;
	}
}
