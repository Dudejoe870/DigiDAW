#include "ui/registry.h"

#include "ui/digidaw.h"

namespace DigiDAW::UI
{
	Registry::Registry(MainApplication* pApp)
	{
		this->pApp = pApp;

		SciterSetGlobalAsset(new DigiDAW());
	}
}
