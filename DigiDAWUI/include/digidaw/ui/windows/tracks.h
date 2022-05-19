#pragma once

#include "digidaw/ui/window.h"
#include "digidaw/ui/ui_state.h"

namespace DigiDAW::UI::Windows
{
	class Tracks : public Window
	{
	private:
		std::shared_ptr<UIState> state;

		void RenderTrackChannelStrip(const std::string& name, std::shared_ptr<Core::Audio::TrackState::Track>& track, bool evenTrack);
	public:
		Tracks(bool open, std::shared_ptr<UIState>& state);

		void Render();
		std::string GetName();
	};
}
