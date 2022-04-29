#include "ui.h"

namespace DigiDAW::UI
{
    UI::UI(std::shared_ptr<DigiDAW::Audio::Engine>& audioEngine)
    {
        this->audioEngine = audioEngine;
    }

	void UI::Render()
	{
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Settings"))
                    showSettingsWindow = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // For Development purposes...
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);
        // ...

        RenderSettingsWindow();
	}

    void UI::RenderSettingsWindow()
    {
        if (showSettingsWindow)
        {
            ImGui::Begin("Settings", &showSettingsWindow);
            {

            }
            ImGui::End();
        }
    }

    ImVec4 UI::GetClearColor()
    {
        return clearColor;
    }
}
