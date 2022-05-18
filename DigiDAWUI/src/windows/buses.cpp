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

	void Buses::Render()
	{
        if (open)
        {
            if (ImGui::Begin(GetName().c_str(), &open, ImGuiWindowFlags_HorizontalScrollbar))
            {
                const std::vector<Core::Audio::Mixer::ChannelInfo>& outputChannels = state->audioEngine->mixer.GetOutputInfo().channels;
                if (outputChannels.size() > 0)
                {
                    if (outputChannels.size() >= 2)
                        Util::DrawAudioMeterStereo("##output_audio_meter_layout",
                            Util::DecibelToPercentage(outputChannels[0].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(outputChannels[1].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(outputChannels[0].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(outputChannels[1].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            false, false, state->audioMeterStyle);
                    else
                        Util::DrawAudioMeter("##output_audio_meter_layout",
                            Util::DecibelToPercentage(outputChannels[0].rms, state->audioEngine->mixer.minimumDecibelLevel),
                            Util::DecibelToPercentage(outputChannels[0].peak, state->audioEngine->mixer.minimumDecibelLevel),
                            false, state->audioMeterStyle);
                }
            }
            ImGui::End();
        }
	}
}
