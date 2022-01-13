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

		currentOutputDevice = audioBackend->getDefaultOutputDevice();
		currentInputDevice = audioBackend->getDefaultInputDevice();
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

	ReturnCode Engine::getDevices(std::vector<Engine::AudioDevice>& dest)
	{
		const unsigned int deviceCount = audioBackend->getDeviceCount();
		for (unsigned int i = 0; i < deviceCount; ++i)
			dest.push_back(Engine::AudioDevice(audioBackend->getDeviceInfo(i), audioBackend->getCurrentApi(), i));

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

	ReturnCode Engine::setCurrentOutputDevice(unsigned int device)
	{
		stopEngine();
		currentOutputDevice = device;
		// TODO: Reopen the stream with the new output device if we already had one open before.
		return ReturnCode::Success;
	}

	ReturnCode Engine::setCurrentInputDevice(unsigned int device)
	{
		stopEngine();
		currentInputDevice = device;
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

	ReturnCode Engine::changeBackend(RtAudio::Api api)
	{
		stopEngine();
		delete audioBackend.release();

		audioBackend = std::make_unique<RtAudio>(api);

		currentOutputDevice = audioBackend->getDefaultOutputDevice();
		currentInputDevice = audioBackend->getDefaultInputDevice();

		return ReturnCode::Success;
	}

	ReturnCode Engine::stopEngine()
	{
		if (audioBackend->isStreamOpen()) audioBackend->closeStream();
		return ReturnCode::Success;
	}
}
