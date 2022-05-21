#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "digidaw/ui/ui.h"

#include "res/resources.h"

#include "imgui-knobs.h"

namespace DigiDAW::UI
{
    UI::UI(std::shared_ptr<Core::Audio::Engine>& audioEngine)
        : currentGuiStyle(ImGui::GetStyle())
    {
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigDockingWithShift = true;

        io.IniFilename = "layout.ini";

        // Setup Fonts
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 19.0f, &fontConfig);

        ImFont* fontHeader1 = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 35.0f, &fontConfig);
        ImFont* fontHeader2 = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::poppins_light_ttf,
            DigiDAW::UI::Resources::poppins_light_ttf_size, 28.0f, &fontConfig);

        static const ImWchar iconRange[] = { 0xf000, 0xffff, 0 };
        ImFont* iconFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)DigiDAW::UI::Resources::fa_solid_ttf,
            DigiDAW::UI::Resources::fa_solid_ttf_size, 18.0f, &fontConfig, iconRange);
        io.Fonts->Build();

        state = std::make_shared<UIState>(fontHeader1, fontHeader2, iconFont, audioEngine, "settings.ini");

        settingsWindow = std::make_unique<Windows::Settings>(false, state);

        effectsChainWindow = std::make_unique<Windows::EffectsChain>(true, state);
        timelineWindow = std::make_unique<Windows::Timeline>(true, state);
        tracksWindow = std::make_unique<Windows::Tracks>(true, state);
        busesWindow = std::make_unique<Windows::Buses>(true, state);

        // Finally, start the audio engine.
        state->audioEngine->StartEngine();
    }

	void UI::Render()
	{
        clearColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        RenderMenuBars();
        RenderDockspace();

        // For Development purposes...
        //ImGui::ShowDemoWindow();
        // ...

        settingsWindow->Render();

        effectsChainWindow->Render();
        timelineWindow->Render();
        tracksWindow->Render();
        busesWindow->Render();
	}

    inline void UI::InitializeDockspace(ImGuiID dockspace, ImGuiDockNodeFlags dockspaceFlags, ImVec2 size)
    {
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, size);

        ImGuiID mainId = dockspace;
        ImGuiID bottomId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Down, 0.30f, nullptr, &mainId);
        ImGuiID topId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Up, 0.25f, nullptr, &mainId);
        ImGuiID bottomRightId = ImGui::DockBuilderSplitNode(bottomId, ImGuiDir_Right, 0.25f, nullptr, &bottomId);

        ImGui::DockBuilderDockWindow(timelineWindow->GetName().c_str(), mainId);
        ImGui::DockBuilderDockWindow(effectsChainWindow->GetName().c_str(), topId);
        ImGui::DockBuilderDockWindow(tracksWindow->GetName().c_str(), bottomId);
        ImGui::DockBuilderDockWindow(busesWindow->GetName().c_str(), bottomRightId);

        ImGui::DockBuilderFinish(dockspace);
    }

    inline void UI::RenderDockspace()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

            // Set the Dockspace size and factor in the height of the status bar 
            // (For some reason viewport->WorkPos and viewport->WorkSize leave space at the top)
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size - ImVec2(0.0f, ImGui::GetFrameHeight()));
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin(dockspaceWindowTitle, nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground);
            ImGui::PopStyleVar();
            {
                ImGuiID mainDockspace = ImGui::GetID(mainWindowDockspace);
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

    inline void UI::RenderMenuBars()
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
                            settingsWindow->open = true;
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("View"))
                    {
                        ImGui::MenuItem(effectsChainWindow->GetName().c_str(), nullptr, &effectsChainWindow->open);
                        ImGui::MenuItem(tracksWindow->GetName().c_str(), nullptr, &tracksWindow->open);
                        ImGui::MenuItem(busesWindow->GetName().c_str(), nullptr, &busesWindow->open);
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("Meters"))
                    {
                        if (ImGui::MenuItem("Reset All Clipping Indicators"))
                            state->audioEngine->mixer.ResetClippingIndicators();
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
                    ImGui::PushFont(state->iconFont);
                    if (state->audioEngine->IsStreamRunning() && state->audioEngine->IsStreamOpen())
                    {
                        ImGui::TextUnformatted((const char*)u8"\uf028");
                        ImGui::PopFont();
                    }
                    else
                    {
                        ImGui::TextUnformatted((const char*)u8"\uf6a9");
                        ImGui::PopFont();
                        if (ImGui::Button("Restart Audio Engine"))
                            state->audioEngine->StartEngine();
                    }

                    Util::TextRightAlign(
                        fmt::format("Current API: {}   Sample Rate: {}hz   Buffer Size: {} Samples",
                            state->audioEngine->GetAPIDisplayName(state->audioEngine->GetCurrentAPI()),
                            state->audioEngine->GetCurrentSampleRate(),
                            state->audioEngine->GetCurrentBufferSize()).c_str(),
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
