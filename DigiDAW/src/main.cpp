#include "main.h"

#include "ui/common.h"

#include "ui/registry.h"

#include "ui/resources.cpp"

namespace DigiDAW
{
	MainApplication::MainApplication()
	{
		audioEngine = std::make_shared<Audio::Engine>(RtAudio::Api::UNSPECIFIED);

        sciter::archive::instance().open(aux::elements_of(resources));

		UI::Registry registry(this);
	}
}
