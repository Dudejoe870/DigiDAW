#include "digidaw/ui/ui.h"

#include "digidaw/ui/gui_util.h"
#include "digidaw/ui/timer.h"

#include <fmt/core.h>

#include "res/resources.h"

namespace DigiDAW::UI
{
    UI::UI(std::shared_ptr<Core::Audio::Engine>& audioEngine)
        : currentGuiStyle(ImGui::GetStyle()), settingsFile("settings.ini")
    {
        this->audioEngine = audioEngine;

        ImGuiIO& io = ImGui::GetIO();

        io.IniFilename = "layout.ini";

        // Setup Fonts
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 19.0f, &fontConfig);

        fontHeader1 = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 35.0f, &fontConfig);

        // Setup Styles
        ImGuiStyle darkStyle;
        ImGuiStyle lightStyle;
        ImGui::StyleColorsDark(&darkStyle);
        ImGui::StyleColorsLight(&lightStyle);

        ModifyStyle(darkStyle);
        ModifyStyle(lightStyle);

        styles = 
        {
            Style("Dark", darkStyle),
            Style("Light", lightStyle)
        };

        // Load state from settings.ini
        SetupStateFromSettings();

        // Finally, start the audio engine.
        audioEngine->StartEngine();
    }

    void UI::SetupStateFromSettings()
    {
        settingsFile.read(settingsStructure); // Read the ini structure.

        // Set the current style.
        try
        {
            currentStyle = std::stoi(settingsStructure["UI"]["style"]);
        }
        catch (std::invalid_argument&)
        {
            currentStyle = 0; // if it isn't an integer, reset it.
        }

        // Verify the style to make sure it's within range of the styles list
        if (currentStyle >= styles.size())
            currentStyle = 0; // Reset it if it isn't.

        currentGuiStyle = styles[currentStyle].guiStyle; // Set the current GUI skin to the one selected.

        // Set the current API.
        RtAudio::Api settingsApi = RtAudio::Api::UNSPECIFIED;
        try
        {
            settingsApi = (RtAudio::Api)std::stoi(settingsStructure["Audio"]["api"]);
        }
        catch (std::invalid_argument&)
        { // Just go to the default if it isn't an integer.
        }

        // Make sure that the API selected is supported and is within the enum range. Otherwise use the default.
        const std::vector<RtAudio::Api>& supportedApis = audioEngine->GetSupportedAPIs();
        if (settingsApi < RtAudio::Api::NUM_APIS &&
            std::find(supportedApis.begin(), supportedApis.end(), settingsApi) != supportedApis.end())
        {
            audioEngine->ChangeBackend(settingsApi);
        }

        // Set the current input device.
        unsigned int settingsInputDevice = audioEngine->GetCurrentInputDevice();
        if (settingsStructure["Audio"]["inputDevice"] == "None")
            settingsInputDevice = -1;
        else
            settingsInputDevice = GetDeviceByName(settingsStructure["Audio"]["inputDevice"]);
        audioEngine->SetCurrentInputDevice(settingsInputDevice);

        // Set the current output device.
        unsigned int settingsOutputDevice = audioEngine->GetCurrentOutputDevice();
        if (settingsStructure["Audio"]["outputDevice"] == "None")
            settingsOutputDevice = -1;
        else
            settingsOutputDevice = GetDeviceByName(settingsStructure["Audio"]["outputDevice"]);
        audioEngine->SetCurrentOutputDevice(settingsOutputDevice);

        // Set the sample rate by default to the fastest supported sample rate.
        const std::vector<unsigned int>& supportedSampleRates = audioEngine->GetSupportedSampleRates();
        unsigned int settingsSampleRate = (supportedSampleRates.size() > 0) ? supportedSampleRates[supportedSampleRates.size()-1] : -1;
        // Try to load the sample rate from the ini file.
        try
        {
            settingsSampleRate = (unsigned int)std::stoi(settingsStructure["Audio"]["sampleRate"]);
        }
        catch (std::invalid_argument&)
        { // If it can't convert it, then use the default.
        }

        // Make sure what we've picked is a supported sample rate, and if it is, set the current sample rate to it.
        if (std::find(supportedSampleRates.begin(), supportedSampleRates.end(), settingsSampleRate) != supportedSampleRates.end())
            audioEngine->SetCurrentSampleRate(settingsSampleRate);

        // Set the current buffer size.
        try
        {
            audioEngine->SetCurrentBufferSize(std::stoi(settingsStructure["Audio"]["bufferSize"]));
        }
        catch (std::invalid_argument&)
        { // Just use the Audio Engine default if it isn't an integer.
        }

        SaveSettings(); // Save all the current settings to the ini file (just incase we had to reset anything due to errors)
    }

    unsigned int UI::GetDeviceByName(std::string name)
    {
        std::vector<Core::Audio::Engine::AudioDevice> devices = audioEngine->GetDevices();
        for (Core::Audio::Engine::AudioDevice& device : devices)
        {
            if (device.info.name == name)
                return device.index;
        }
        return -1;
    }

    void UI::ModifyStyle(ImGuiStyle& style)
    {
        // Get rid of all transparency that can look weird when dragging a secondary window out of the main one.
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        style.Colors[ImGuiCol_TitleBgCollapsed].w = 1.0f;

        for (int i = 0; i < ImGuiCol_COUNT; ++i)
        {
            // Make all the channels grayscale.
            float grayscale = (style.Colors[i].x + style.Colors[i].y + style.Colors[i].z) / 3.0f;
            style.Colors[i].x = grayscale;
            style.Colors[i].y = grayscale;
            style.Colors[i].z = grayscale;
        }

        // No Frame border, and a little frame rounding.
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 0.0f;

        // Align the Window title to the center, add window border, and no window collapse arrow.
        // (double-click to collapse still works without the no collapse flag active per window)
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowBorderSize = 1.0f;
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }

    void UI::SaveSettings()
    {
        // Audio
        settingsStructure["Audio"]["bufferSize"] = fmt::format("{}", audioEngine->GetCurrentBufferSize());
        settingsStructure["Audio"]["api"] = fmt::format("{}", (int)audioEngine->GetCurrentAPI());
        settingsStructure["Audio"]["sampleRate"] = fmt::format("{}", audioEngine->GetCurrentSampleRate());

        const std::vector<Core::Audio::Engine::AudioDevice>& devices = audioEngine->GetDevices();
        unsigned int currentInputDevice = audioEngine->GetCurrentInputDevice();
        unsigned int currentOutputDevice = audioEngine->GetCurrentOutputDevice();
        settingsStructure["Audio"]["inputDevice"] = 
            (currentInputDevice != -1 && currentInputDevice < devices.size()) ? devices[currentInputDevice].info.name : "None";
        settingsStructure["Audio"]["outputDevice"] =
            (currentOutputDevice != -1 && currentOutputDevice < devices.size()) ? devices[currentOutputDevice].info.name : "None";

        // UI
        settingsStructure["UI"]["style"] = fmt::format("{}", currentStyle);

        settingsFile.write(settingsStructure); // Write the settings to the ini file.
    }

	void UI::Render()
	{
        // For Development purposes...
        ImGui::ShowDemoWindow();
        // ...

        RenderSettingsWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                        shouldExit = true;
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

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
            if (ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoDecoration))
            {
                ImGui::End();
            }
        }
        ImGui::PopStyleVar();
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
                        const std::vector<RtAudio::Api>& supportedAPIs = audioEngine->GetSupportedAPIs();
                        RtAudio::Api currentAPI = audioEngine->GetCurrentAPI();
                        if (ImGui::BeginCombo("Audio API", audioEngine->GetAPIDisplayName(currentAPI).c_str()))
                        {
                            for (RtAudio::Api api : supportedAPIs)
                            {
                                if (ImGui::Selectable(audioEngine->GetAPIDisplayName(api).c_str(), audioEngine->GetCurrentAPI() == api))
                                {
                                    audioEngine->ChangeBackend(api);
                                    SaveSettings();
                                }
                            }

                            ImGui::EndCombo();
                        }

                        ImGui::Separator();

                        // Devices
                        const std::vector<Core::Audio::Engine::AudioDevice>& devices = audioEngine->GetDevices();

                        // Input Device Dropdown
                        unsigned int currentInput = audioEngine->GetCurrentInputDevice();
                        if (ImGui::BeginCombo("Input Device", (currentInput != -1) ? devices[currentInput].info.name.c_str() : "None"))
                        {
                            if (ImGui::Selectable("None", currentInput == -1))
                                audioEngine->SetCurrentInputDevice(-1);

                            for (Core::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.probed && device.info.inputChannels > 0)
                                {
                                    if (ImGui::Selectable(device.info.name.c_str(), currentInput == device.index))
                                    {
                                        audioEngine->SetCurrentInputDevice(device.index);
                                        SaveSettings();
                                    }
                                }
                            }

                            ImGui::EndCombo();
                        }

                        // Output Device Dropdown
                        unsigned int currentOutput = audioEngine->GetCurrentOutputDevice();
                        if (ImGui::BeginCombo("Output Device", (currentOutput != -1) ? devices[currentOutput].info.name.c_str() : "None"))
                        {
                            if (ImGui::Selectable("None", currentOutput == -1))
                                audioEngine->SetCurrentOutputDevice(-1);

                            for (Core::Audio::Engine::AudioDevice device : devices)
                            {
                                if (device.info.probed && device.info.outputChannels > 0)
                                {
                                    if (ImGui::Selectable(device.info.name.c_str(), currentOutput == device.index))
                                    {
                                        audioEngine->SetCurrentOutputDevice(device.index);
                                        SaveSettings();
                                    }
                                }
                            }

                            ImGui::EndCombo();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Test Tone"))
                        {
                            audioEngine->mixer.StartTestTone();
                            Timer::AddTimer((unsigned long long)TimerID::TestToneTimer, 1.0f, 
                                [&]()
                                {
                                    audioEngine->mixer.EndTestTone();
                                });
                        }

                        ImGui::Separator();

                        // Sample Rate Dropdown
                        const std::vector<unsigned int>& supportedSampleRates = audioEngine->GetSupportedSampleRates();
                        unsigned int currentSampleRate = audioEngine->GetCurrentSampleRate();
                        if (ImGui::BeginCombo("Sample Rate", fmt::format("{}hz", currentSampleRate).c_str()))
                        {
                            for (unsigned int rate : supportedSampleRates)
                            {
                                if (ImGui::Selectable(fmt::format("{}hz", rate).c_str(), currentSampleRate == rate))
                                {
                                    audioEngine->SetCurrentSampleRate(rate);
                                    SaveSettings();
                                }
                            }

                            ImGui::EndCombo();
                        }

                        // Buffer Size Dropdown
                        unsigned int currentBufferSize = audioEngine->GetCurrentBufferSize();
                        float currentLatencyMS = ((float)currentBufferSize / (float)currentSampleRate) * 1000.0f;
                        if (ImGui::BeginCombo("Buffer Size", 
                            fmt::format("{} Samples ({:.2}ms)", currentBufferSize, currentLatencyMS).c_str()))
                        {
                            for (unsigned int bufferSize = 32; bufferSize <= 4096; bufferSize *= 2)
                            {
                                float latencyMS = ((float)bufferSize / (float)currentSampleRate) * 1000.0f;
                                if (ImGui::Selectable(
                                    fmt::format("{} Samples ({:.2}ms)", bufferSize, latencyMS).c_str(), currentBufferSize == bufferSize))
                                {
                                    audioEngine->SetCurrentBufferSize(bufferSize);
                                    SaveSettings();
                                }
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

    bool UI::ShouldExit()
    {
        return shouldExit;
    }
}
