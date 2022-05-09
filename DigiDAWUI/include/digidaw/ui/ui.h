#pragma once

#include "imgui.h"
#include "ImGuiFileBrowser.h"

#include "mini/ini.h"

#include "digidaw/ui/gui_util.h"

#include <digidaw/core/audio/engine.h>

#include <memory>
#include <string>

#include <fmt/core.h>

namespace DigiDAW::UI
{
	class UI
	{
	private:
		enum class TimerID : unsigned long long
		{
			TestToneTimer = 0
		};

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

		// UI State
		bool showDemoWindow = true;
		bool showSettingsWindow = false;
		bool showTracksWindow = true;
		bool showBusesWindow = true;
		bool showTimelineWindow = true;

		ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		std::shared_ptr<Core::Audio::Engine> audioEngine;
		
		ImFont* fontHeader1 = nullptr;
		ImFont* fontHeader2 = nullptr;
		ImFont* iconFont = nullptr;

		std::vector<Style> styles;
		unsigned int currentStyle;

		Util::AudioMeterStyle audioMeterStyle;

		ImGuiStyle& currentGuiStyle;

		mINI::INIFile settingsFile;
		mINI::INIStructure settingsStructure;

		imgui_addons::ImGuiFileBrowser fileBrowser;

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
				out = (unsigned int)std::stoul(settingsStructure[section][name]);
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

		void SetupStateFromSettings();

		unsigned int GetDeviceByName(std::string name);

		void ModifyStyle(ImGuiStyle& style, bool withBorder);
		void SaveSettings();

		void RenderTracksWindow();
		void RenderBusesWindow();
		void RenderTimelineWindow();

		void RenderSettingsWindow();
		
		void InitializeDockspace(ImGuiID dockspace, ImGuiDockNodeFlags dockspaceFlags, ImVec2 size);
		void RenderDockspace();
		void RenderMenuBars();

		bool hasDockspaceBeenInitialized = false;

		bool shouldExit = false;

		const std::string mainWindowDockspace = "MainWindowDock";
		const std::string dockspaceWindowTitle = "DockSpace";
		const std::string settingsWindowTitle = "Settings";
		const std::string tracksWindowTitle = "Tracks";
		const std::string busesWindowTitle = "Buses";
		const std::string timelineWindowTitle = "Timeline";
	public:
		UI(std::shared_ptr<Core::Audio::Engine>& audioEngine);

		void Render();
		ImVec4 GetClearColor();

		bool ShouldExit();
	};
}
