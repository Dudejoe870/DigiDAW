#include "ui/audioengine.h"

namespace DigiDAW::UI
{
	AudioEngine::AudioEngine(MainApplication* pApp)
	{
		this->pApp = pApp;

		audioFormats = sciter::value::make_map();
		audioFormats.set_item("SINT8", (int)RTAUDIO_SINT8);
		audioFormats.set_item("SINT16", (int)RTAUDIO_SINT16);
		audioFormats.set_item("SINT24", (int)RTAUDIO_SINT24);
		audioFormats.set_item("SINT32", (int)RTAUDIO_SINT32);
		audioFormats.set_item("FLOAT32", (int)RTAUDIO_FLOAT32);
		audioFormats.set_item("FLOAT64", (int)RTAUDIO_FLOAT64);
	}

	sciter::astring AudioEngine::getAPIDisplayName(int api)
	{
		return pApp->audioEngine->getAPIDisplayName((RtAudio::Api)api);
	}

	std::vector<int> AudioEngine::getSupportedAPIs()
	{
		std::vector<RtAudio::Api> supportedAPIs = pApp->audioEngine->getSupportedAPIs();

		std::vector<int> ret;
		for (RtAudio::Api api : supportedAPIs) ret.push_back((int)api);
		return ret;
	}

	int AudioEngine::getCurrentAPI()
	{
		return (int)pApp->audioEngine->getCurrentAPI();
	}

	void AudioEngine::changeBackend(int api)
	{
		pApp->audioEngine->changeBackend((RtAudio::Api)api);
	}

	std::vector<sciter::value> AudioEngine::queryDevices()
	{
		std::vector<Audio::Engine::AudioDevice> devices = pApp->audioEngine->getDevices();

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

	std::vector<int> AudioEngine::getSupportedSampleRates()
	{
		std::vector<unsigned int> supportedSampleRates;
		pApp->audioEngine->getSupportedSampleRates(supportedSampleRates);

		std::vector<int> ret;
		for (unsigned int rate : supportedSampleRates) ret.push_back((int)rate);
		return ret;
	}
}
