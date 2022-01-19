#include "main.h"

#include "ui/common.h"

#include "ui/registry.h"

#include "ui/resources.cpp"

#include "audio/trackstate.h"

namespace DigiDAW
{
	MainApplication::MainApplication()
	{
		audioEngine = std::make_shared<Audio::Engine>(RtAudio::Api::UNSPECIFIED);

		Audio::TrackState::BusIdentifier mainBus = audioEngine->trackState.addBus(
			Audio::TrackState::Bus(
				Audio::TrackState::ChannelNumber::Stereo,
				0.0f, 0.0f,
				std::vector<Audio::TrackState::BusOutput> {  },
				std::vector<std::vector<unsigned int>>
				{
					std::vector<unsigned int> { 0 },
					std::vector<unsigned int> { 1 }
				})); // Make a Bus that outputs to the first two channels of the output device.

		audioEngine->trackState.addTrack(
			Audio::TrackState::Track(
				Audio::TrackState::ChannelNumber::Mono,
				-25.0f, 0.0f,
				std::vector<Audio::TrackState::BusOutput>
				{
					Audio::TrackState::BusOutput(mainBus, std::vector<std::vector<unsigned int>>
					{
						std::vector<unsigned int> { 0, 1 }
					})
				})); // Make a Mono Track that outputs both it's channels to the L and R channels of the mainBus.

        sciter::archive::instance().open(aux::elements_of(resources));

		UI::Registry registry(this);
	}
}
