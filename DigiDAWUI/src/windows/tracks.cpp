#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "digidaw/ui/windows/tracks.h"

#include "imgui-knobs.h"

namespace DigiDAW::UI::Windows
{
	Tracks::Tracks(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
		this->state = state;
	}

    std::string Tracks::GetName()
    {
        return "Tracks";
    }

    inline void Tracks::RenderTrackChannelStrip(const std::string& name, std::shared_ptr<Core::Audio::TrackState::Track>& track, bool evenTrack)
    {
        Util::DrawChannelStripBackground(evenTrack);

        ImGui::BeginVertical(std::string("##" + name).c_str(), ImVec2(Util::channelStripWidth, 0.0f), 0.5f);
        {
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            Util::DrawMixableControls("Track Name", state->audioEngine, track, state->audioMeterStyle, 6.0f);

            // TODO: Draw Bus Outputs
        }
        ImGui::EndVertical();
    }

	void Tracks::Render()
	{
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        {
            if (ImGui::Begin(GetName().c_str(), &open, ImGuiWindowFlags_HorizontalScrollbar))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                {
                    ImGui::BeginHorizontal("##track_channel_strips", ImVec2(0.0f, 0.0f), 0.0f);
                    {
                        unsigned int trackIndex = 0;
                        for (std::shared_ptr<Core::Audio::TrackState::Track>& track : state->audioEngine->trackState.GetAllTracks())
                        {
                            RenderTrackChannelStrip("track" + trackIndex, track, trackIndex % 2 == 0);
                            ++trackIndex;
                        }
                    }
                    ImGui::EndHorizontal();
                }
                ImGui::PopStyleVar();
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();
	}
}
