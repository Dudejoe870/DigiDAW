#include "registry.h"

#include "ui/digidaw.h"

namespace DigiDAW::UI
{
	void Registry::RegisterAllAssets()
	{
		SciterSetGlobalAsset(new DigiDAW());
	}
}
