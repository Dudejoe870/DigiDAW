#pragma once

#include "imgui.h"

#include "mini/ini.h"
#include "digidaw/ui/gui_util.h"

#include <digidaw/core/audio/engine.h>

namespace DigiDAW::UI
{
	struct UIState
	{
    private:
        void ModifyStyle(ImGuiStyle& style, bool withBorder)
        {
            // Get rid of all transparency that can look weird when dragging a secondary window out of the main one.
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            style.Colors[ImGuiCol_PopupBg].w = 1.0f;
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

            const ImVec4 windowBg = style.Colors[ImGuiCol_WindowBg];
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(windowBg.x * 1.15f, windowBg.y * 1.15f, windowBg.z * 1.15f, 1.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(windowBg.x * 0.95f, windowBg.y * 0.95f, windowBg.z * 0.95f, 1.0f);

            style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

            style.FrameRounding = 3.0f;
            style.FrameBorderSize = (withBorder) ? 1.0f : 0.0f;
            style.FramePadding = ImVec2(7.0f, 2.0f);

            style.ItemSpacing = ImVec2(6.0f, 3.0f);
            style.LayoutAlign = 0.0f;

            style.TabRounding = 8.0f;
            style.TabBorderSize = 0.0f;

            style.GrabRounding = 8.0f;

            // Align the Window title to the center, add window border, and no window collapse arrow.
            // (double-click to collapse still works without the no collapse flag active per window)
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowBorderSize = 1.0f;
            style.WindowMenuButtonPosition = ImGuiDir_None;
        }

        void CreateLowContrastTheme(ImGuiStyle& style)
        {
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];

            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);

            style.Colors[ImGuiCol_Header] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);

            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);

            style.Colors[ImGuiCol_Button] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);

            style.Colors[ImGuiCol_Tab] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

            ModifyStyle(style, false);

            const ImVec4 windowBg = style.Colors[ImGuiCol_WindowBg];
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(windowBg.x * 1.25f, windowBg.y * 1.25f, windowBg.z * 1.25f, 1.0f);
        }

        unsigned int GetDeviceByName(std::string name)
        {
            std::vector<Core::Audio::Engine::AudioDevice> devices = audioEngine->GetDevices();
            for (Core::Audio::Engine::AudioDevice& device : devices)
            {
                if (device.info.name == name)
                    return device.index;
            }
            return -1;
        }

        bool SettingsTryGetFloat(const std::string& section, const std::string& name, float& out)
        {
            try
            {
                out = std::stof(settingsStructure[section][name]);
                return true;
            }
            catch (std::invalid_argument&)
            {
                return false;
            }
        }

        bool SettingsTryGetBool(const std::string& section, const std::string& name, bool& out)
        {
            std::string setting = settingsStructure[section][name];
            std::transform(setting.begin(), setting.end(), setting.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (setting == "true")
                out = true;
            else if (setting == "false")
                out = false;
            else
                return false;
            return true;
        }

        bool SettingsTryGetInt(const std::string& section, const std::string& name, int& out)
        {
            try
            {
                out = std::stoi(settingsStructure[section][name]);
                return true;
            }
            catch (std::invalid_argument&)
            {
                return false;
            }
        }

        bool SettingsTryGetUInt(const std::string& section, const std::string& name, unsigned int& out)
        {
            try
            {
                out = static_cast<unsigned int>(std::stoul(settingsStructure[section][name]));
                return true;
            }
            catch (std::invalid_argument&)
            {
                return false;
            }
        }

        bool SettingsTryGetColor(const std::string& section, const std::string& name, ImVec4& out)
        {
            bool ret = false;
            ret |= SettingsTryGetFloat(section, name + "_R", out.x);
            ret |= SettingsTryGetFloat(section, name + "_G", out.y);
            ret |= SettingsTryGetFloat(section, name + "_B", out.z);
            return ret;
        }

        template <typename T>
        void SettingsSave(const std::string& section, const std::string& name, const T& in)
        {
            settingsStructure[section][name] = fmt::format("{}", in);
        }

        template <>
        void SettingsSave<ImVec4>(const std::string& section, const std::string& name, const ImVec4& in)
        {
            SettingsSave(section, name + "_R", in.x);
            SettingsSave(section, name + "_G", in.y);
            SettingsSave(section, name + "_B", in.z);
        }

        void SetupStateFromSettings()
        {
            settingsFile.read(settingsStructure); // Read the ini structure.

            // Set the current style.
            if (!SettingsTryGetUInt("UI", "style", currentStyle))
                currentStyle = 0; // if it isn't an integer, reset it.

            // Verify the style to make sure it's within range of the styles list
            if (currentStyle >= styles.size())
                currentStyle = 0; // Reset it if it isn't.

            ImGui::GetStyle() = styles[currentStyle].guiStyle; // Set the current GUI skin to the one selected.

            SettingsTryGetBool("UI", "audioMeter_segmented", audioMeterStyle.segmented);
            SettingsTryGetBool("UI", "audioMeter_rounded", audioMeterStyle.rounded);

            SettingsTryGetInt("UI", "audioMeter_lineSegments", audioMeterStyle.lineSegments);
            SettingsTryGetFloat("UI", "audioMeter_lineAlpha", audioMeterStyle.lineAlpha);

            SettingsTryGetInt("UI", "audioMeter_meterWidth", audioMeterStyle.meterWidth);

            SettingsTryGetInt("UI", "audioMeter_stereoMeterSpacing", audioMeterStyle.stereoMeterSpacing);

            SettingsTryGetColor("UI", "audioMeter_lowRangeColor", audioMeterStyle.lowRangeColor);
            SettingsTryGetColor("UI", "audioMeter_midRangeColor", audioMeterStyle.midRangeColor);
            SettingsTryGetColor("UI", "audioMeter_highRangeColor", audioMeterStyle.highRangeColor);

            // Set the current API.
            unsigned int settingsApi = static_cast<unsigned int>(RtAudio::Api::UNSPECIFIED);
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
            unsigned int settingsSampleRate = (supportedSampleRates.size() > 0) ? supportedSampleRates[supportedSampleRates.size() - 1] : -1;
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
    public:
        struct Style
        {
            std::string name;
            ImGuiStyle guiStyle;

            Style(std::string name, ImGuiStyle& guiStyle)
            {
                this->name = name;
                this->guiStyle = guiStyle;
            }
        };

        std::vector<Style> styles;

		ImFont* fontHeader1 = nullptr;
		ImFont* fontHeader2 = nullptr;
		ImFont* iconFont = nullptr;

		std::shared_ptr<Core::Audio::Engine> audioEngine;

		unsigned int currentStyle;

		Util::AudioMeterStyle audioMeterStyle;

		mINI::INIFile settingsFile;
		mINI::INIStructure settingsStructure;

        void SaveSettings()
        {
            // Audio
            SettingsSave("Audio", "bufferSize", audioEngine->GetCurrentBufferSize());
            SettingsSave("Audio", "api", static_cast<int>(audioEngine->GetCurrentAPI()));
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

            SettingsSave("UI", "audioMeter_meterWidth", audioMeterStyle.meterWidth);

            SettingsSave("UI", "audioMeter_stereoMeterSpacing", audioMeterStyle.stereoMeterSpacing);

            SettingsSave("UI", "audioMeter_lowRangeColor", audioMeterStyle.lowRangeColor);
            SettingsSave("UI", "audioMeter_midRangeColor", audioMeterStyle.midRangeColor);
            SettingsSave("UI", "audioMeter_highRangeColor", audioMeterStyle.highRangeColor);

            SettingsSave("UI", "audioMeter_activeClipColor", audioMeterStyle.activeClipColor);

            settingsFile.write(settingsStructure); // Write the settings to the ini file.
        }

		UIState(ImFont* fontHeader1, ImFont* fontHeader2, ImFont* iconFont, std::shared_ptr<Core::Audio::Engine>& audioEngine, const std::string& iniFilename)
            : settingsFile(iniFilename)
        {
            this->fontHeader1 = fontHeader1;
            this->fontHeader2 = fontHeader2;
            this->iconFont = iconFont;

            this->audioEngine = audioEngine;

            // Setup Styles
            ImGuiStyle darkStyle;
            ImGuiStyle lowContrastLightStyle;
            ImGuiStyle highContrastLightStyle;
            ImGui::StyleColorsDark(&darkStyle);
            ImGui::StyleColorsLight(&lowContrastLightStyle);
            ImGui::StyleColorsLight(&highContrastLightStyle);

            ModifyStyle(darkStyle, false);
            ModifyStyle(highContrastLightStyle, true);

            // Make the Low Contrast theme
            CreateLowContrastTheme(lowContrastLightStyle);

            styles =
            {
                Style("Dark", darkStyle),
                Style("Low Contrast Light", lowContrastLightStyle),
                Style("High Contrast Light", highContrastLightStyle)
            };

            SetupStateFromSettings();
		}
	};
}
