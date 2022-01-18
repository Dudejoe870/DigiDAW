#include "audio/mixer.h"

#include "audio/engine.h"

#ifdef __AVX__
#include <immintrin.h>
#else
#error AVX isn't available... cannot compile.
#endif

namespace DigiDAW::Audio
{
	Mixer::Mixer(Engine& audioEngine) 
		: audioEngine(audioEngine)
	{
		this->doTestTone = false;
		this->testToneStartTime = 0.0;
		this->currentTime = 0.0;

		audioEngine.trackState.registerUpdateTracksHandler([=]() { updateTrackBuffers(); });
		audioEngine.trackState.registerUpdateBusesHandler ([=]() { updateBusBuffers(); });
	}

	void Mixer::updateTrackBuffers()
	{
		const std::vector<TrackState::TrackIdentifier>& tracks = audioEngine.trackState.getAllTracks();

		trackBuffers.clear();

		for (const TrackState::TrackIdentifier& track : tracks)
		{
			TrackState::Track trackInfo = audioEngine.trackState.getTrack(track);
			trackBuffers[track] = MixBuffer(audioEngine.getRealBufferSize(), (unsigned int)trackInfo.nChannels);
		}
	}

	void Mixer::updateBusBuffers()
	{
		const std::vector<TrackState::BusIdentifier>& buses = audioEngine.trackState.getAllBuses();

		busBuffers.clear();

		for (const TrackState::BusIdentifier& bus : buses)
		{
			TrackState::Bus busInfo = audioEngine.trackState.getBus(bus);
			busBuffers[bus] = MixBuffer(audioEngine.getRealBufferSize(), (unsigned int)busInfo.nChannels);
		}
	}

	void Mixer::updateCurrentTime(double time)
	{
		this->currentTime = time;
	}

	void Mixer::mix(
		float* outputBuffer,
		float* inputBuffer, 
		double time, 
		unsigned int nFrames,
		unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate)
	{
		currentTime = time;

		if (!doTestTone)
		{
		}
		else
		{
			std::vector<float> monoOutput;
			for (unsigned int frame = 0; frame < nFrames; ++frame)
			{
				double sampleTime = (time + ((double)frame / (double)sampleRate)) - testToneStartTime;
				monoOutput.push_back((float)((0.10 * (std::clamp(1.0 - sampleTime, 0.0, 1.0))) * std::sin(2 * pi * 440.0 * sampleTime)));
			}

			std::memcpy(&outputBuffer[0 * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
			if (nOutChannels > 1) std::memcpy(&outputBuffer[1 * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
		}
	}

	void Mixer::startTestTone()
	{
		testToneStartTime = currentTime;
		doTestTone = true;
	}

	void Mixer::endTestTone()
	{
		testToneStartTime = 0.0;
		doTestTone = false;
	}
}
