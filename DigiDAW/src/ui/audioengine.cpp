#include "ui/audioengine.h"

namespace DigiDAW::UI
{
	AudioEngine::AudioEngine(MainApplication* pApp)
	{
		this->pApp = pApp;

		for (int i = 0; i < RtAudio::Api::NUM_APIS; ++i)
		{
			std::string apiName = "";
			pApp->audioEngine->getAPIName((RtAudio::Api)i, apiName);
			apiMapping.push_back(apiName);
		}
	}

	std::vector<sciter::astring> AudioEngine::getAPIEnum()
	{
		return apiMapping;
	}

	sciter::astring AudioEngine::getAPIDisplayName(int api)
	{
		std::string ret;
		pApp->audioEngine->getAPIDisplayName((RtAudio::Api)api, ret);
		return ret;
	}

	std::vector<int> AudioEngine::getSupportedAPIs()
	{
		std::vector<RtAudio::Api> supportedAPIs;
		pApp->audioEngine->getSupportedAPIs(supportedAPIs);

		std::vector<int> ret;
		for (RtAudio::Api api : supportedAPIs) ret.push_back((int)api);
		return ret;
	}

	int AudioEngine::getCurrentAPI()
	{
		RtAudio::Api ret;
		pApp->audioEngine->getCurrentAPI(ret);
		return (int)ret;
	}

	void AudioEngine::changeBackend(int api)
	{
		pApp->audioEngine->changeBackend((RtAudio::Api)api);
	}

	std::vector<sciter::value> AudioEngine::queryDevices()
	{
		std::vector<Audio::Engine::AudioDevice> devices;
		pApp->audioEngine->getDevices(devices);

		std::vector<sciter::value> ret;
		for (Audio::Engine::AudioDevice device : devices)
		{
			sciter::value jsDevice = sciter::value::make_map();

			jsDevice.set_item("index", device.index);
			jsDevice.set_item("api", (int)device.backend);
			jsDevice.set_item("probed", device.info.probed);
			jsDevice.set_item("name", device.info.name);
			jsDevice.set_item("outputChannels", device.info.outputChannels);
			jsDevice.set_item("inputChannels", device.info.inputChannels);
			jsDevice.set_item("duplexChannels", device.info.duplexChannels);
			jsDevice.set_item("isDefaultOutput", device.info.isDefaultOutput);
			jsDevice.set_item("isDefaultInput", device.info.isDefaultInput);
			jsDevice.set_item("sampleRates", device.info.sampleRates);
			jsDevice.set_item("currentSampleRate", device.info.currentSampleRate);
			jsDevice.set_item("preferredSampleRate", device.info.preferredSampleRate);
			jsDevice.set_item("nativeFormats", (int)device.info.nativeFormats);

			ret.push_back(jsDevice);
		}

		return ret;
	}
}
