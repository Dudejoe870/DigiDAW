#pragma once

#include "digidaw/ui/window.h"
#include "digidaw/ui/ui_state.h"

namespace DigiDAW::UI::Windows
{
	class Buses : public Window
	{
	private:
		std::shared_ptr<UIState> state;
	public:
		Buses(bool open, std::shared_ptr<UIState>& state);

		void RenderBusChannelStrip(const std::string& name, std::shared_ptr<Core::Audio::TrackState::Bus>& bus, bool evenTrack);

		void Render();
		std::string GetName();
	};
}
