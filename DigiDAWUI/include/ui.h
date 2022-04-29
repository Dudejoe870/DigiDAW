#pragma once

#include "imgui.h"

#include <audio/engine.h>

#include <memory>

namespace DigiDAW::UI
{
	class UI
	{
	private:
		// UI State
		bool showDemoWindow = true;
		bool showSettingsWindow = false;

		ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		std::shared_ptr<DigiDAW::Audio::Engine> audioEngine;
	
		void RenderSettingsWindow();
	public:
		UI(std::shared_ptr<DigiDAW::Audio::Engine>& audioEngine);

		void Render();
		ImVec4 GetClearColor();
	};
}
