#include "digidaw/ui/windows/timeline.h"

namespace DigiDAW::UI::Windows
{
	Timeline::Timeline(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
		this->state = state;
	}

	std::string Timeline::GetName()
	{
		return "Timeline";
	}

	void Timeline::Render()
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
