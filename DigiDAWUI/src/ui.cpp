#include "ui.h"

#include <fmt/core.h>

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
                if (ImGui::BeginTabBar("SettingsTabBar"))
                {
                    if (ImGui::BeginTabItem("Audio")) // Audio Settings
                    {
                        ImGui::Text("Audio Settings");
                        ImGui::Separator();

                        // API Dropdown
                        std::vector<RtAudio::Api> supportedAPIs = audioEngine->GetSupportedAPIs();
                        RtAudio::Api currentAPI = audioEngine->GetCurrentAPI();
                        if (ImGui::BeginCombo("Audio API", audioEngine->GetAPIDisplayName(currentAPI).c_str()))
                        {
                            for (RtAudio::Api api : supportedAPIs)
                            {
                                if (ImGui::Selectable(audioEngine->GetAPIDisplayName(api).c_str(), audioEngine->GetCurrentAPI() == api))
                                    audioEngine->ChangeBackend(api);
                            }

                            ImGui::EndCombo();
                        }

                        // Devices
                        std::vector<DigiDAW::Audio::Engine::AudioDevice> devices = audioEngine->GetDevices();

                        // Input Device Dropdown
                        unsigned int currentInput = audioEngine->GetCurrentInputDevice();
                        if (ImGui::BeginCombo("Input Device", (currentInput != -1) ? devices[currentInput].info.name.c_str() : "None"))
                        {
                            for (DigiDAW::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.inputChannels > 0)
                                    if (ImGui::Selectable(device.info.name.c_str(), currentInput == device.index))
                                        audioEngine->SetCurrentInputDevice(device.index);
                            }

                            ImGui::EndCombo();
                        }

                        // Output Device Dropdown
                        unsigned int currentOutput = audioEngine->GetCurrentOutputDevice();
                        if (ImGui::BeginCombo("Output Device", (currentOutput != -1) ? devices[currentOutput].info.name.c_str() : "None"))
                        {
                            for (DigiDAW::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.outputChannels > 0)
                                    if (ImGui::Selectable(device.info.name.c_str(), currentOutput == device.index))
                                        audioEngine->SetCurrentOutputDevice(device.index);
                            }

                            ImGui::EndCombo();
                        }

                        ImGui::Separator();

                        // Sample Rate Dropdown
                        std::vector<unsigned int> supportedSampleRates = audioEngine->GetCurrentSupportedSampleRates();
                        unsigned int currentSampleRate = audioEngine->GetCurrentSampleRate();
                        if (ImGui::BeginCombo("Sample Rate", fmt::format("{}hz", currentSampleRate).c_str()))
                        {
                            for (unsigned int rate : supportedSampleRates)
                            {
                                if (ImGui::Selectable(fmt::format("{}hz", rate).c_str(), currentSampleRate == rate))
                                    audioEngine->SetCurrentSampleRate(rate);
                            }

                            ImGui::EndCombo();
                        }

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
        }
    }

    ImVec4 UI::GetClearColor()
    {
        return clearColor;
    }
}
