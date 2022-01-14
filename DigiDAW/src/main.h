#pragma once

#include "stdcommon.h"

#include "audio/engine.h"

namespace DigiDAW
{
	class MainApplication
	{
	public:
		std::shared_ptr<Audio::Engine> audioEngine;

		MainApplication();
	};
}
