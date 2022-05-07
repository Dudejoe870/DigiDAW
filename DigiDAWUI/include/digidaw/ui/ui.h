#pragma once

#include "imgui.h"

#include "mini/ini.h"

#include <digidaw/core/audio/engine.h>

#include <memory>
#include <string>

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
		ImFont* iconFont = nullptr;

		std::vector<Style> styles;
		unsigned int currentStyle;

		ImGuiStyle& currentGuiStyle;

		mINI::INIFile settingsFile;
		mINI::INIStructure settingsStructure;

		void SetupStateFromSettings();

		unsigned int GetDeviceByName(std::string name);

		void ModifyStyle(ImGuiStyle& style);
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
