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

		ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		std::shared_ptr<Core::Audio::Engine> audioEngine;
		
		ImFont* fontHeader1 = nullptr;

		std::vector<Style> styles;
		unsigned int currentStyle;

		ImGuiStyle& currentGuiStyle;

		mINI::INIFile settingsFile;
		mINI::INIStructure settingsStructure;

		void SetupStateFromSettings();

		unsigned int GetDeviceByName(std::string name);

		void ModifyStyle(ImGuiStyle& style);
		void SaveSettings();
		void RenderSettingsWindow();

		bool shouldExit = false;
	public:
		UI(std::shared_ptr<Core::Audio::Engine>& audioEngine);

		void Render();
		ImVec4 GetClearColor();

		bool ShouldExit();
	};
}
