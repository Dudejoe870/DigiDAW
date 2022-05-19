#include "digidaw/ui/windows/settings.h"

#include "digidaw/ui/gui_util.h"
#include "digidaw/ui/timer.h"

namespace DigiDAW::UI::Windows
{
	Settings::Settings(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
        this->state = state;
	}

    std::string Settings::GetName()
    {
        return "Settings";
    }

    inline void Settings::RenderAudioTab()
    {
        ImGui::PushFont(state->fontHeader1);
        Util::TextCentered("Audio Settings");
        ImGui::PopFont();

        ImGui::Separator();

        // API Dropdown
        const std::vector<RtAudio::Api>& supportedAPIs = state->audioEngine->GetSupportedAPIs();
        RtAudio::Api currentAPI = state->audioEngine->GetCurrentAPI();
        if (ImGui::BeginCombo("Audio API", state->audioEngine->GetAPIDisplayName(currentAPI).c_str()))
        {
            for (RtAudio::Api api : supportedAPIs)
            {
                if (ImGui::Selectable(state->audioEngine->GetAPIDisplayName(api).c_str(), state->audioEngine->GetCurrentAPI() == api))
                {
                    state->audioEngine->ChangeBackend(api);
                    state->SaveSettings();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Devices
        const std::vector<Core::Audio::Engine::AudioDevice>& devices = state->audioEngine->GetDevices();

        // Input Device Dropdown
        unsigned int currentInput = state->audioEngine->GetCurrentInputDevice();
        if (ImGui::BeginCombo("Input Device", (currentInput != -1) ? devices[currentInput].info.name.c_str() : "None"))
        {
            if (ImGui::Selectable("None", currentInput == -1))
                state->audioEngine->SetCurrentInputDevice(-1);

            for (Core::Audio::Engine::AudioDevice device : devices)
            {
                if (device.info.probed && device.info.inputChannels > 0)
                {
                    if (ImGui::Selectable(device.info.name.c_str(), currentInput == device.index))
                    {
                        state->audioEngine->SetCurrentInputDevice(device.index);
                        state->SaveSettings();
                    }
                }
            }

            ImGui::EndCombo();
        }

        // Output Device Dropdown
        unsigned int currentOutput = state->audioEngine->GetCurrentOutputDevice();
        if (ImGui::BeginCombo("Output Device", (currentOutput != -1) ? devices[currentOutput].info.name.c_str() : "None"))
        {
            if (ImGui::Selectable("None", currentOutput == -1))
                state->audioEngine->SetCurrentOutputDevice(-1);

            for (Core::Audio::Engine::AudioDevice device : devices)
            {
                if (device.info.probed && device.info.outputChannels > 0)
                {
                    if (ImGui::Selectable(device.info.name.c_str(), currentOutput == device.index))
                    {
                        state->audioEngine->SetCurrentOutputDevice(device.index);
                        state->SaveSettings();
                    }
                }
            }

            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Test Tone"))
        {
            state->audioEngine->mixer.StartTestTone();
            Timer::AddTimer(0, 1.0f,
                [&]()
                {
                    state->audioEngine->mixer.EndTestTone();
                });
        }

        ImGui::Separator();

        // Sample Rate Dropdown
        const std::vector<unsigned int>& supportedSampleRates = state->audioEngine->GetSupportedSampleRates();
        unsigned int currentSampleRate = state->audioEngine->GetCurrentSampleRate();
        if (ImGui::BeginCombo("Sample Rate", fmt::format("{}hz", currentSampleRate).c_str()))
        {
            for (unsigned int rate : supportedSampleRates)
            {
                if (ImGui::Selectable(fmt::format("{}hz", rate).c_str(), currentSampleRate == rate))
                {
                    state->audioEngine->SetCurrentSampleRate(rate);
                    state->SaveSettings();
                }
            }

            ImGui::EndCombo();
        }

        // Buffer Size Dropdown
        unsigned int currentBufferSize = state->audioEngine->GetCurrentBufferSize();
        float currentLatencyMS = ((float)currentBufferSize / (float)currentSampleRate) * 1000.0f;
        if (ImGui::BeginCombo("Buffer Size",
            fmt::format("{} Samples ({:.2f}ms)", currentBufferSize, currentLatencyMS).c_str()))
        {
            for (unsigned int bufferSize = 32; bufferSize <= 4096; bufferSize *= 2)
            {
                float latencyMS = ((float)bufferSize / (float)currentSampleRate) * 1000.0f;
                if (ImGui::Selectable(
                    fmt::format("{} Samples ({:.2f}ms)", bufferSize, latencyMS).c_str(), currentBufferSize == bufferSize))
                {
                    state->audioEngine->SetCurrentBufferSize(bufferSize);
                    state->SaveSettings();
                }
            }

            ImGui::EndCombo();
        }
    }

    inline void Settings::RenderUITab()
    {
        ImGui::PushFont(state->fontHeader1);
        Util::TextCentered("UI Settings");
        ImGui::PopFont();

        ImGui::Separator();

        // Style Dropdown
        if (ImGui::BeginCombo("Style", state->styles[state->currentStyle].name.c_str()))
        {
            for (int i = 0; i < state->styles.size(); ++i)
            {
                if (ImGui::Selectable(state->styles[i].name.c_str(), state->currentStyle == i))
                {
                    state->currentStyle = i;
                    state->SaveSettings();
                    ImGui::GetStyle() = state->styles[state->currentStyle].guiStyle;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::Separator();

        bool saveSettings = false;

        ImGui::PushFont(state->fontHeader2);
        Util::TextCentered("Audio Meter Settings");
        ImGui::PopFont();

        saveSettings |= ImGui::Checkbox("Segmented", &state->audioMeterStyle.segmented);
        ImGui::SameLine();
        saveSettings |= ImGui::Checkbox("Rounded", &state->audioMeterStyle.rounded);

        ImGui::BeginDisabled(!state->audioMeterStyle.segmented);
        saveSettings |= ImGui::SliderInt("Line Segments", &state->audioMeterStyle.lineSegments, 4, 128);
        saveSettings |= ImGui::SliderFloat("Line Opacity", &state->audioMeterStyle.lineAlpha, 0.0f, 1.0f);
        ImGui::EndDisabled();

        saveSettings |= ImGui::SliderInt("Stereo Meter Spacing", &state->audioMeterStyle.stereoMeterSpacing, 1, 5);

        saveSettings |= ImGui::SliderInt("Meter Width", &state->audioMeterStyle.meterWidth, 10, 16);

        Util::TextCentered("Colors");
        ImGui::Separator();

        saveSettings |= ImGui::ColorEdit3("Low Range Segment", &state->audioMeterStyle.lowRangeColor.x, ImGuiColorEditFlags_NoInputs);
        saveSettings |= ImGui::ColorEdit3("Mid Range Segment", &state->audioMeterStyle.midRangeColor.x, ImGuiColorEditFlags_NoInputs);
        saveSettings |= ImGui::ColorEdit3("High Range Segment", &state->audioMeterStyle.highRangeColor.x, ImGuiColorEditFlags_NoInputs);
        saveSettings |= ImGui::ColorEdit3("Clip Indicator", &state->audioMeterStyle.activeClipColor.x, ImGuiColorEditFlags_NoInputs);

        if (ImGui::Button("Reset To Defaults"))
        {
            state->audioMeterStyle = Util::AudioMeterStyle();
            saveSettings = true;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Resets Audio Meter settings to default.");
            ImGui::EndTooltip();
        }

        ImGui::BeginHorizontal("##audio_meters_layout");
        {
            Util::DrawAudioMeterStereo("##stereo_audio_meter_layout", 1.0f, 0.5f, 1.0f, 0.7f, true, false, state->audioMeterStyle);
            Util::DrawAudioMeter("##mono_audio_meter_layout", 0.7f, 0.8f, true, state->audioMeterStyle);
        }
        ImGui::EndHorizontal();

        if (saveSettings)
            state->SaveSettings();
    }

    void Settings::Render()
    {
        if (open)
        {
            ImGui::SetNextWindowSize(ImVec2(1024.0f, 780.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(915.0f, 780.0f), ImVec2(10000000.0f, 10000000.0f));
            if (ImGui::Begin(GetName().c_str(), &open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
            {
                if (ImGui::BeginTabBar("SettingsTabBar"))
                {
                    if (ImGui::BeginTabItem("Audio")) // Audio Settings
                    {
                        RenderAudioTab();
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("UI")) // UI Settings
                    {
                        RenderUITab();
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
        }
    }
}
