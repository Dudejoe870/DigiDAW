#pragma once

#include "ui/common.h"

#include "main.h"

namespace DigiDAW::UI
{
	class Registry
	{
	private:
		MainApplication* pApp;
	public:
		Registry(MainApplication* pApp);
	};
}
