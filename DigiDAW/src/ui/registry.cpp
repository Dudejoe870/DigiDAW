#include "ui/registry.h"

#include "ui/digidaw.h"
#include "ui/audioengine.h"

namespace DigiDAW::UI
{
	Registry::Registry(MainApplication* pApp)
	{
		this->pApp = pApp;

		SciterSetGlobalAsset(new DigiDAW());
		SciterSetGlobalAsset(new AudioEngine(pApp));
	}
}
