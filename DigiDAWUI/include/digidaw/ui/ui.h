#pragma once

#include "imgui.h"
#include "ImGuiFileBrowser.h"

#include "digidaw/ui/gui_util.h"

#include "digidaw/ui/ui_state.h"

#include "digidaw/ui/windows/settings.h"
#include "digidaw/ui/windows/timeline.h"
#include "digidaw/ui/windows/tracks.h"
#include "digidaw/ui/windows/buses.h"

#include <digidaw/core/audio/engine.h>

#include <memory>
#include <string>

namespace DigiDAW::UI
{
	class UI
	{
	private:
		ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGuiStyle& currentGuiStyle;

		std::shared_ptr<UIState> state;

		std::unique_ptr<Windows::Settings> settingsWindow;

		std::unique_ptr<Windows::Timeline> timelineWindow;
		std::unique_ptr<Windows::Tracks> tracksWindow;
		std::unique_ptr<Windows::Buses> busesWindow;
		
		void InitializeDockspace(ImGuiID dockspace, ImGuiDockNodeFlags dockspaceFlags, ImVec2 size);
		void RenderDockspace();
		void RenderMenuBars();

		bool hasDockspaceBeenInitialized = false;

		bool shouldExit = false;

		const char* mainWindowDockspace = "MainWindowDock";
		const char* dockspaceWindowTitle = "DockSpace";
	public:
		UI(std::shared_ptr<Core::Audio::Engine>& audioEngine);

		void Render();
		ImVec4 GetClearColor();

		bool ShouldExit();
	};
}
