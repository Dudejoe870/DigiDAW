#pragma once

#include "imgui.h"

#include "mini/ini.h"

#include <audio/engine.h>

#include <memory>
#include <string>

namespace DigiDAW::UI
{
	class UI
	{
	private:
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

		ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		std::shared_ptr<DigiDAW::Audio::Engine> audioEngine;
		
		ImFont* fontHeader1 = nullptr;

		std::vector<Style> styles;
		unsigned int currentStyle;

		ImGuiStyle& currentGuiStyle;

		mINI::INIFile settingsFile;
		mINI::INIStructure settingsStructure;

		void SaveSettings();
		void RenderSettingsWindow();
	public:
		UI(std::shared_ptr<DigiDAW::Audio::Engine>& audioEngine);

		void Render();
		ImVec4 GetClearColor();
	};
}
