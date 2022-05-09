#include "digidaw/ui/ui.h"
#include "digidaw/ui/timer.h"

#include "res/resources.h"

#include "imgui-knobs.h"
#include "imgui_internal.h"

namespace DigiDAW::UI
{
    UI::UI(std::shared_ptr<Core::Audio::Engine>& audioEngine)
        : currentGuiStyle(ImGui::GetStyle()), settingsFile("settings.ini")
    {
        this->audioEngine = audioEngine;

        ImGuiIO& io = ImGui::GetIO();

        io.ConfigDockingWithShift = true;

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
        fontHeader2 = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 28.0f, &fontConfig);

        static const ImWchar iconRange[] = { 0xf000, 0xffff, 0 };
        iconFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::fa_solid_ttf,
            DigiDAW::UI::Resources::fa_solid_ttf_size, 18.0f, &fontConfig, iconRange);
        io.Fonts->Build();

        // Setup Styles
        ImGuiStyle darkStyle;
        ImGuiStyle lightStyle;
        ImGui::StyleColorsDark(&darkStyle);
        ImGui::StyleColorsLight(&lightStyle);

        ModifyStyle(darkStyle, false);
        ModifyStyle(lightStyle, true);

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
        if (!SettingsTryGetUInt("UI", "style", currentStyle))
            currentStyle = 0; // if it isn't an integer, reset it.

        // Verify the style to make sure it's within range of the styles list
        if (currentStyle >= styles.size())
            currentStyle = 0; // Reset it if it isn't.

        currentGuiStyle = styles[currentStyle].guiStyle; // Set the current GUI skin to the one selected.

        SettingsTryGetBool("UI", "audioMeter_segmented", audioMeterStyle.segmented);
        SettingsTryGetBool("UI", "audioMeter_rounded", audioMeterStyle.rounded);

        SettingsTryGetInt("UI", "audioMeter_lineSegments", audioMeterStyle.lineSegments);
        SettingsTryGetFloat("UI", "audioMeter_lineAlpha", audioMeterStyle.lineAlpha);

        SettingsTryGetInt("UI", "audioMeter_stereoMeterSpacing", audioMeterStyle.stereoMeterSpacing);

        SettingsTryGetColor("UI", "audioMeter_lowRangeColor", audioMeterStyle.lowRangeColor);
        SettingsTryGetColor("UI", "audioMeter_midRangeColor", audioMeterStyle.midRangeColor);
        SettingsTryGetColor("UI", "audioMeter_highRangeColor", audioMeterStyle.highRangeColor);

        // Set the current API.
        unsigned int settingsApi = (unsigned int)RtAudio::Api::UNSPECIFIED;
        SettingsTryGetUInt("Audio", "api", settingsApi);

        // Make sure that the API selected is supported and is within the enum range. Otherwise use the default.
        const std::vector<RtAudio::Api>& supportedApis = audioEngine->GetSupportedAPIs();
        if (settingsApi < RtAudio::Api::NUM_APIS &&
            std::find(supportedApis.begin(), supportedApis.end(), settingsApi) != supportedApis.end())
        {
            audioEngine->ChangeBackend((RtAudio::Api)settingsApi);
        }

        // Set the current input device.
        unsigned int settingsInputDevice = audioEngine->GetCurrentInputDevice();
        if (settingsStructure["Audio"]["inputDevice"] == "None")
            settingsInputDevice = -1;
        else
            settingsInputDevice = GetDeviceByName(settingsStructure["Audio"]["inputDevice"]);
        if (!settingsStructure["Audio"]["inputDevice"].empty()) audioEngine->SetCurrentInputDevice(settingsInputDevice);

        // Set the current output device.
        unsigned int settingsOutputDevice = audioEngine->GetCurrentOutputDevice();
        if (settingsStructure["Audio"]["outputDevice"] == "None")
            settingsOutputDevice = -1;
        else
            settingsOutputDevice = GetDeviceByName(settingsStructure["Audio"]["outputDevice"]);
        if (!settingsStructure["Audio"]["outputDevice"].empty()) audioEngine->SetCurrentOutputDevice(settingsOutputDevice);

        // Set the sample rate by default to the fastest supported sample rate.
        const std::vector<unsigned int>& supportedSampleRates = audioEngine->GetSupportedSampleRates();
        unsigned int settingsSampleRate = (supportedSampleRates.size() > 0) ? supportedSampleRates[supportedSampleRates.size()-1] : -1;
        // Try to load the sample rate from the ini file.
        SettingsTryGetUInt("Audio", "sampleRate", settingsSampleRate);

        // Make sure what we've picked is a supported sample rate, and if it is, set the current sample rate to it.
        if (std::find(supportedSampleRates.begin(), supportedSampleRates.end(), settingsSampleRate) != supportedSampleRates.end())
            audioEngine->SetCurrentSampleRate(settingsSampleRate);

        // Set the current buffer size.
        unsigned int bufferSize = 0;
        if (SettingsTryGetUInt("Audio", "bufferSize", bufferSize))
            audioEngine->SetCurrentBufferSize(bufferSize);

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

    void UI::ModifyStyle(ImGuiStyle& style, bool withBorder)
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

        style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

        style.FrameRounding = 3.0f;
        style.FrameBorderSize = (withBorder) ? 1.0f : 0.0f;
        style.FramePadding = ImVec2(7.0f, 2.0f);

        style.ItemSpacing = ImVec2(6.0f, 3.0f);

        style.TabRounding = 8.0f;
        style.TabBorderSize = 0.0f;

        style.GrabRounding = 8.0f;

        // Align the Window title to the center, add window border, and no window collapse arrow.
        // (double-click to collapse still works without the no collapse flag active per window)
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowBorderSize = 1.0f;
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }

    void UI::SaveSettings()
    {
        // Audio
        SettingsSave("Audio", "bufferSize", audioEngine->GetCurrentBufferSize());
        SettingsSave("Audio", "api", (int)audioEngine->GetCurrentAPI());
        SettingsSave("Audio", "sampleRate", audioEngine->GetCurrentSampleRate());

        const std::vector<Core::Audio::Engine::AudioDevice>& devices = audioEngine->GetDevices();
        unsigned int currentInputDevice = audioEngine->GetCurrentInputDevice();
        unsigned int currentOutputDevice = audioEngine->GetCurrentOutputDevice();
        SettingsSave("Audio", "inputDevice",
            (currentInputDevice != -1 && currentInputDevice < devices.size()) ? devices[currentInputDevice].info.name : "None");
        SettingsSave("Audio", "outputDevice",
            (currentOutputDevice != -1 && currentOutputDevice < devices.size()) ? devices[currentOutputDevice].info.name : "None");

        // UI
        SettingsSave("UI", "style", currentStyle);

        SettingsSave("UI", "audioMeter_segmented", audioMeterStyle.segmented);
        SettingsSave("UI", "audioMeter_rounded", audioMeterStyle.rounded);

        SettingsSave("UI", "audioMeter_lineSegments", audioMeterStyle.lineSegments);
        SettingsSave("UI", "audioMeter_lineAlpha", audioMeterStyle.lineAlpha);

        SettingsSave("UI", "audioMeter_stereoMeterSpacing", audioMeterStyle.stereoMeterSpacing);

        SettingsSave("UI", "audioMeter_lowRangeColor", audioMeterStyle.lowRangeColor);
        SettingsSave("UI", "audioMeter_midRangeColor", audioMeterStyle.midRangeColor);
        SettingsSave("UI", "audioMeter_highRangeColor", audioMeterStyle.highRangeColor);

        SettingsSave("UI", "audioMeter_activeClipColor", audioMeterStyle.activeClipColor);

        settingsFile.write(settingsStructure); // Write the settings to the ini file.
    }

	void UI::Render()
	{
        clearColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        RenderMenuBars();
        RenderDockspace();

        // For Development purposes...
        //ImGui::ShowDemoWindow();
        // ...
        
        RenderTracksWindow();
        RenderBusesWindow();
        RenderTimelineWindow();

        RenderSettingsWindow();
	}

    void UI::RenderSettingsWindow()
    {
        if (showSettingsWindow)
        {
            ImGui::SetNextWindowSize(ImVec2(1024.0f, 768.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(915.0f, 138.0f), ImVec2(10000000.0f, 10000000.0f));
            ImGui::Begin(settingsWindowTitle.c_str(), &showSettingsWindow,
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

                        ImGui::Separator();

                        bool saveSettings = false;

                        ImGui::PushFont(fontHeader2);
                        Util::TextCentered("Audio Meter Settings");
                        ImGui::PopFont();

                        saveSettings |= ImGui::Checkbox("Segmented", &audioMeterStyle.segmented);
                        ImGui::SameLine();
                        saveSettings |= ImGui::Checkbox("Rounded", &audioMeterStyle.rounded);

                        ImGui::BeginDisabled(!audioMeterStyle.segmented);
                        saveSettings |= ImGui::SliderInt("Line Segments", &audioMeterStyle.lineSegments, 4, 128);
                        saveSettings |= ImGui::SliderFloat("Line Opacity", &audioMeterStyle.lineAlpha, 0.0f, 1.0f);
                        ImGui::EndDisabled();

                        saveSettings |= ImGui::SliderInt("Stereo Meter Spacing", &audioMeterStyle.stereoMeterSpacing, 1, 6);

                        Util::TextCentered("Colors");
                        ImGui::Separator();
                        
                        saveSettings |= ImGui::ColorEdit3("Low Range Segment", &audioMeterStyle.lowRangeColor.x, ImGuiColorEditFlags_NoInputs);
                        saveSettings |= ImGui::ColorEdit3("Mid Range Segment", &audioMeterStyle.midRangeColor.x, ImGuiColorEditFlags_NoInputs);
                        saveSettings |= ImGui::ColorEdit3("High Range Segment", &audioMeterStyle.highRangeColor.x, ImGuiColorEditFlags_NoInputs);
                        saveSettings |= ImGui::ColorEdit3("Clip Indicator", &audioMeterStyle.activeClipColor.x, ImGuiColorEditFlags_NoInputs);

                        if (ImGui::Button("Reset To Defaults"))
                        {
                            audioMeterStyle = Util::AudioMeterStyle();
                            saveSettings = true;
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted("Resets Audio Meter settings to default.");
                            ImGui::EndTooltip();
                        }

                        Util::DrawAudioMeterStereo(1.0f, 0.5f, 1.0f, 0.7f, true, false, audioMeterStyle);

                        if (saveSettings)
                            SaveSettings();

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
        }
    }

    void UI::RenderTracksWindow()
    {
        if (showTracksWindow)
        {
            if (ImGui::Begin(tracksWindowTitle.c_str(), &showTracksWindow))
            {
                const std::vector<Core::Audio::Mixer::ChannelInfo>& outputChannels = audioEngine->mixer.GetOutputInfo().channels;
                if (outputChannels.size() > 0)
                {
                    if (outputChannels.size() >= 2)
                        Util::DrawAudioMeterStereo(
                            Util::DecibelToPercentage(outputChannels[0].rmsAmplitude), 
                            Util::DecibelToPercentage(outputChannels[1].rmsAmplitude),
                            Util::DecibelToPercentage(outputChannels[0].peakAmplitude),
                            Util::DecibelToPercentage(outputChannels[1].peakAmplitude),
                            false, false, audioMeterStyle);
                    else
                        Util::DrawAudioMeter(
                            Util::DecibelToPercentage(outputChannels[0].rmsAmplitude),
                            Util::DecibelToPercentage(outputChannels[0].peakAmplitude),
                            false, audioMeterStyle);
                }
            }
            ImGui::End();
        }
    }

    void UI::RenderBusesWindow()
    {
        if (showBusesWindow)
        {
            if (ImGui::Begin(busesWindowTitle.c_str(), &showBusesWindow))
            {

            }
            ImGui::End();
        }
    }

    void UI::RenderTimelineWindow()
    {
        if (showTimelineWindow)
        {
            if (ImGui::Begin(timelineWindowTitle.c_str(), &showTimelineWindow))
            {

            }
            ImGui::End();
        }
    }

    void UI::InitializeDockspace(ImGuiID dockspace, ImGuiDockNodeFlags dockspaceFlags, ImVec2 size)
    {
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, size);

        ImGuiID mainId = dockspace;
        ImGuiID bottomId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Down, 0.3f, nullptr, &mainId);
        ImGuiID bottomRightId = ImGui::DockBuilderSplitNode(bottomId, ImGuiDir_Right, 0.25f, nullptr, &bottomId);

        ImGui::DockBuilderDockWindow(timelineWindowTitle.c_str(), mainId);
        ImGui::DockBuilderDockWindow(tracksWindowTitle.c_str(), bottomId);
        ImGui::DockBuilderDockWindow(busesWindowTitle.c_str(), bottomRightId);

        ImGui::DockBuilderFinish(dockspace);
    }

    void UI::RenderDockspace()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin(dockspaceWindowTitle.c_str(), nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground);
            ImGui::PopStyleVar();
            {
                ImGuiID mainDockspace = ImGui::GetID(mainWindowDockspace.c_str());
                ImGui::DockSpace(mainDockspace, ImVec2(0.0f, 0.0f), dockspaceFlags);

                if (!hasDockspaceBeenInitialized)
                {
                    hasDockspaceBeenInitialized = true;

                    // Setup Dockspace
                    InitializeDockspace(mainDockspace, dockspaceFlags, viewport->Size);
                }
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();
    }

    void UI::RenderMenuBars()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        {
            if (ImGui::BeginViewportSideBar("##MainMenuBar", nullptr, ImGuiDir_Up, ImGui::GetFrameHeight(),
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
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
                    ImGui::EndMenuBar();
                }
            }
            ImGui::End();

            if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, ImGui::GetFrameHeight(),
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    ImGui::PushFont(iconFont);
                    if (audioEngine->IsStreamRunning() && audioEngine->IsStreamOpen())
                    {
                        ImGui::TextUnformatted((const char*)u8"\uf028");
                        ImGui::PopFont();
                    }
                    else
                    {
                        ImGui::TextUnformatted((const char*)u8"\uf6a9");
                        ImGui::PopFont();
                        if (ImGui::Button("Restart Audio Engine"))
                            audioEngine->StartEngine();
                    }

                    Util::TextRightAlign(
                        fmt::format("Current API: {}   Sample Rate: {}hz   Buffer Size: {} Samples",
                            audioEngine->GetAPIDisplayName(audioEngine->GetCurrentAPI()),
                            audioEngine->GetCurrentSampleRate(),
                            audioEngine->GetCurrentBufferSize()).c_str(),
                        10.0f); // Info Text
                    ImGui::EndMenuBar();
                }
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();
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
