#include "ui/audiomixer.h"

namespace DigiDAW::UI
{
	AudioMixer::AudioMixer(MainApplication* pApp)
	{
		this->pApp = pApp;
	}

	void AudioMixer::startTestTone()
	{
		pApp->audioEngine->mixer.startTestTone();
	}

	void AudioMixer::endTestTone()
	{
		pApp->audioEngine->mixer.endTestTone();
	}
}
