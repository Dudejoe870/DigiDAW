#include "ui.h"

#include "gui_util.h"

#include <fmt/core.h>

#include "res/resources.h"

namespace DigiDAW::UI
{
    UI::UI(std::shared_ptr<DigiDAW::Audio::Engine>& audioEngine)
        : currentGuiStyle(ImGui::GetStyle()), settingsFile("settings.ini")
    {
        this->audioEngine = audioEngine;

        ImGuiIO& io = ImGui::GetIO();

        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 19.0f, &fontConfig);

        fontHeader1 = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 35.0f, &fontConfig);

        ImGuiStyle darkStyle;
        ImGuiStyle lightStyle;
        ImGui::StyleColorsDark(&darkStyle);
        ImGui::StyleColorsLight(&lightStyle);

        darkStyle.WindowRounding = 0.0f;
        darkStyle.Colors[ImGuiCol_WindowBg].w = 1.0f;
        lightStyle.WindowRounding = 0.0f;
        lightStyle.Colors[ImGuiCol_WindowBg].w = 1.0f;

        styles = 
        {
            Style("Dark", darkStyle),
            Style("Light", lightStyle)
        };

        settingsFile.read(settingsStructure);

        try
        {
            currentStyle = stoi(settingsStructure["UI"]["style"]);
        }
        catch (std::invalid_argument&)
        {
            currentStyle = 0;
            SaveSettings();
        }

        if (currentStyle >= styles.size())
        {
            currentStyle = 0;
            SaveSettings();
        }

        currentGuiStyle = styles[currentStyle].guiStyle;
    }

    void UI::SaveSettings()
    {
        settingsStructure["UI"]["style"] = fmt::format("{}", currentStyle);
        settingsFile.write(settingsStructure);
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
        ImGui::ShowDemoWindow();
        // ...

        RenderSettingsWindow();
	}

    void UI::RenderSettingsWindow()
    {
        if (showSettingsWindow)
        {
            ImGui::SetNextWindowSize(ImVec2(1024.0f, 768.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(915.0f, 138.0f), ImVec2(10000000.0f, 10000000.0f));
            ImGui::Begin("Settings", &showSettingsWindow, 
                ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
            {
                if (ImGui::BeginTabBar("SettingsTabBar"))
                {
                    if (ImGui::BeginTabItem("Audio")) // Audio Settings
                    {
                        ImGui::PushFont(fontHeader1);
                        Util::TextCentered("Audio Settings");
                        ImGui::PopFont();

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

                        ImGui::Separator();

                        // Devices
                        std::vector<DigiDAW::Audio::Engine::AudioDevice> devices = audioEngine->GetDevices();

                        // Input Device Dropdown
                        unsigned int currentInput = audioEngine->GetCurrentInputDevice();
                        if (ImGui::BeginCombo("Input Device", (currentInput != -1) ? devices[currentInput].info.name.c_str() : "None"))
                        {
                            if (ImGui::Selectable("None", currentInput == -1))
                                audioEngine->SetCurrentInputDevice(-1);

                            for (DigiDAW::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.probed && device.info.inputChannels > 0)
                                    if (ImGui::Selectable(device.info.name.c_str(), currentInput == device.index))
                                        audioEngine->SetCurrentInputDevice(device.index);
                            }

                            ImGui::EndCombo();
                        }

                        // Output Device Dropdown
                        unsigned int currentOutput = audioEngine->GetCurrentOutputDevice();
                        if (ImGui::BeginCombo("Output Device", (currentOutput != -1) ? devices[currentOutput].info.name.c_str() : "None"))
                        {
                            if (ImGui::Selectable("None", currentOutput == -1))
                                audioEngine->SetCurrentOutputDevice(-1);

                            for (DigiDAW::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.probed && device.info.outputChannels > 0)
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

                    if (ImGui::BeginTabItem("UI")) // UI Settings
                    {
                        ImGui::PushFont(fontHeader1);
                        Util::TextCentered("UI Settings");
                        ImGui::PopFont();

                        ImGui::Separator();

                        // Style Dropdown
                        if (ImGui::BeginCombo("Style", styles[currentStyle].name.c_str()))
                        {
                            for (int i = 0; i < styles.size(); ++i)
                            {
                                if (ImGui::Selectable(styles[i].name.c_str(), currentStyle == i))
                                {
                                    currentStyle = i;
                                    SaveSettings();
                                    currentGuiStyle = styles[currentStyle].guiStyle;
                                }
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
