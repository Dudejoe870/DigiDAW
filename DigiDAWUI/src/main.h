#pragma once

#include "audio/engine.h"

#include <memory>

class MainApplication
{
public:
	QPalette darkPalette;

	std::shared_ptr<DigiDAW::Audio::Engine> audioEngine;

	MainApplication();
};
