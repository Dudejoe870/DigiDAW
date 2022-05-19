#include "digidaw/ui/windows/buses.h"

namespace DigiDAW::UI::Windows
{
	Buses::Buses(bool open, std::shared_ptr<UIState>& state)
		: Window(open)
	{
		this->state = state;
	}

	std::string Buses::GetName()
	{
		return "Buses";
	}

    inline void Buses::RenderBusChannelStrip(const std::string& name, std::shared_ptr<Core::Audio::TrackState::Bus>& bus, bool evenTrack)
    {
        Util::DrawChannelStripBackground(evenTrack);

        ImGui::BeginVertical(std::string("##" + name).c_str(), ImVec2(Util::channelStripWidth, 0.0f), 0.5f);
        {
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            Util::DrawMixableControls("Bus Name", state->audioEngine, bus, state->audioMeterStyle, 6.0f);

            // TODO: Draw Output Device Outputs
        }
        ImGui::EndVertical();
    }

	void Buses::Render()
	{
        if (open)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            {
                if (ImGui::Begin(GetName().c_str(), &open, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                    {
                        // Make the output device channel strip disappear when the window is too small.
                        // (We would just use a window size constraint so this doesn't happen,
                        // however size constraints for docked windows isn't currently 
                        // implemented in ImGUI unfortunately)
                        bool shouldDrawOutputDevice = ImGui::GetWindowWidth() > Util::channelStripWidth;

                        ImGui::BeginHorizontal("##buses_layout");
                        {
                            if (ImGui::BeginChild("##bus_channel_strips", 
                                ImVec2(ImGui::GetContentRegionMax().x - (shouldDrawOutputDevice ? Util::channelStripWidth : 0), 0), 
                                false, ImGuiWindowFlags_HorizontalScrollbar))
                            {
                                //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); // Already pushed above
                                {
                                    ImGui::BeginHorizontal("##bus_channel_strips", ImVec2(0.0f, 0.0f), 0.0f);
                                    {
                                        unsigned int busIndex = 0;
                                        for (std::shared_ptr<Core::Audio::TrackState::Bus>& bus : state->audioEngine->trackState.GetAllBuses())
                                        {
                                            RenderBusChannelStrip("bus" + busIndex, bus, busIndex % 2 == 0);
                                            ++busIndex;
                                        }
                                    }
                                    ImGui::EndHorizontal();
                                }
                                //ImGui::PopStyleVar();
                            }
                            ImGui::EndChild();

                            if (shouldDrawOutputDevice)
                            {
                                if (ImGui::BeginChild("##output_device_strip", ImVec2(Util::channelStripWidth, 0)))
                                {
                                    Util::DrawChannelStripBackground(true);
                                    ImGui::BeginVertical("##channel_strip", ImVec2(Util::channelStripWidth, 0.0f), 0.5f);
                                    {
                                        ImGui::Dummy(ImVec2(0.0f, 5.0f));

                                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 3.0f));
                                        {
                                            // We create these Vertical layouts so both layouts contents are centered.
                                            ImGui::BeginVertical("##text_v_layout", ImVec2(Util::channelStripWidth, 0.0f), 0.5f);
                                            {
                                                ImGui::TextUnformatted("Output Device");
                                            }
                                            ImGui::EndVertical();

                                            ImGui::BeginVertical("##audio_meter_v_layout", ImVec2(Util::channelStripWidth, 0.0f), 0.5f);
                                            {
                                                // TODO: Add option to display more channels 
                                                // (the output device can have way more channels than just 2)
                                                const std::vector<Core::Audio::Mixer::ChannelInfo>& outputChannels =
                                                    state->audioEngine->mixer.GetOutputInfo().channels;
                                                if (outputChannels.size() > 0)
                                                {
                                                    if (outputChannels.size() >= 2)
                                                        Util::DrawAudioMeterStereo("##output_audio_meter_layout",
                                                            Util::DecibelToPercentage(outputChannels[0].rms, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            Util::DecibelToPercentage(outputChannels[1].rms, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            Util::DecibelToPercentage(outputChannels[0].peak, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            Util::DecibelToPercentage(outputChannels[1].peak, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            false, false, state->audioMeterStyle);
                                                    else
                                                        Util::DrawAudioMeter("##output_audio_meter_layout",
                                                            Util::DecibelToPercentage(outputChannels[0].rms, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            Util::DecibelToPercentage(outputChannels[0].peak, 
                                                                state->audioEngine->mixer.minimumDecibelLevel),
                                                            false, state->audioMeterStyle);
                                                }
                                            }
                                            ImGui::EndVertical();
                                        }
                                        ImGui::PopStyleVar();
                                    }
                                    ImGui::EndVertical();
                                }
                                ImGui::EndChild();
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
}
