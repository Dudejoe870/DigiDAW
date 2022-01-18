#include "ui/registry.h"

#include "ui/digidaw.h"
#include "ui/audioengine.h"
#include "ui/audiomixer.h"

namespace DigiDAW::UI
{
	Registry::Registry(MainApplication* pApp)
	{
		this->pApp = pApp;

		SciterSetGlobalAsset(new DigiDAW());
		SciterSetGlobalAsset(new AudioEngine(pApp));
		SciterSetGlobalAsset(new AudioMixer(pApp));
	}
}
