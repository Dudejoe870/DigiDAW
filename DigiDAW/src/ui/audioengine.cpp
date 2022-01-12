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
}
