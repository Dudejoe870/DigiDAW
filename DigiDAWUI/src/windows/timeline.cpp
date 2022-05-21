#include "digidaw/ui/windows/timeline.h"

namespace DigiDAW::UI::Windows
{
	Timeline::Timeline(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
		this->state = state;

		windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	}

	std::string Timeline::GetName()
	{
		return "Timeline";
	}

	void Timeline::Render()
	{
		ImGui::SetNextWindowClass(&windowClass);
		if (ImGui::Begin(GetName().c_str(), nullptr, 
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
		{
		}
		ImGui::End();
	}
}
