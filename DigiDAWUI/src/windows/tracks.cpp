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

    inline void Tracks::RenderTrackChannelStrip(const std::string& name, Core::Audio::TrackState::Track& track, bool evenTrack)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImGuiViewport* viewport = ImGui::GetWindowViewport();

        ImVec2 pos = window->DC.CursorPos - ImVec2(0.0f, style.WindowPadding.y);
        ImVec2 size = ImVec2(140.0f, viewport->Size.y);
        ImRect bb(pos, pos + size);

        ImU32 bgColor = evenTrack ? ImGui::GetColorU32(ImGuiCol_TableRowBg) : ImGui::GetColorU32(ImGuiCol_TableRowBgAlt);
        ImGui::RenderFrame(bb.Min, bb.Max, bgColor, false);

        ImGui::BeginVertical(std::string("##" + name).c_str(), ImVec2(size.x, 0.0f), 0.5f);
        {
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 3.0f));
            {
                ImGui::InputTextEx("##name", "Track Name", track.name, 256, ImVec2(100.0f, 0.0f), 0);

                ImGuiKnobs::Knob("Pan", &track.pan, -100.0f, 100.0f, 0.0f, "%.0f",
                    ImGuiKnobVariant_Wiper, 48.0f, ImGuiKnobFlags_DragHorizontal);

                float faderGain = std::powf(10.0f, track.gain / 20.0f);
                ImGui::BeginHorizontal("##fader_meter_layout", ImVec2(0.0f, 0.0f), 0.5f);
                {
                    const float maxSlider = std::powf(10.0f, 6.0f / 20.0f);

                    ImGui::VSliderFloat("##gain", ImVec2(20.0f, Util::audioMeterFullHeight), &faderGain, 0.0f, maxSlider, "",
                        ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);

                    const Core::Audio::Mixer::MixableInfo& trackInfo = state->audioEngine->mixer.GetMixableInfo(track);
                    if (trackInfo.channels.size() == 1)
                    {
                        Util::DrawAudioMeter("##audio_meter_layout",
                            Util::DecibelToPercentage(trackInfo.channels[0].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(trackInfo.channels[0].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            false, state->audioMeterStyle);
                    }
                    else if (trackInfo.channels.size() == 2)
                    {
                        Util::DrawAudioMeterStereo("##audio_meter_layout",
                            Util::DecibelToPercentage(trackInfo.channels[0].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(trackInfo.channels[1].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(trackInfo.channels[0].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(trackInfo.channels[1].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            false, false, state->audioMeterStyle);
                    }
                }
                ImGui::EndHorizontal();

                track.gain = 20.0f * std::log10f(faderGain);
                ImGui::SetNextItemWidth(64.0f);
                ImGui::InputFloat("##gain_input", &track.gain, 0.0f, 0.0f, "%.1fdB");
            }
            ImGui::PopStyleVar();
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
                        for (Core::Audio::TrackState::Track& track : state->audioEngine->trackState.GetAllTracks())
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
