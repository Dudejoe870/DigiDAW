#pragma once

#include "digidaw/ui/window.h"
#include "digidaw/ui/ui_state.h"

namespace DigiDAW::UI::Windows
{
	class EffectsChain : public Window
	{
	private:
		std::shared_ptr<UIState> state;
	public:
		EffectsChain(bool open, std::shared_ptr<UIState>& state);

		void Render();
		std::string GetName();
	};
}
