#include "digidaw/ui/windows/effects_chain.h"

namespace DigiDAW::UI::Windows
{
	EffectsChain::EffectsChain(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
		this->state = state;
	}

	std::string EffectsChain::GetName()
	{
		return "Effects Chain";
	}

	void EffectsChain::Render()
	{
		if (open)
		{
			if (ImGui::Begin(GetName().c_str(), &open))
			{

			}
			ImGui::End();
		}
	}
}
