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

	void Mixer::applyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		// TODO: Vectorize this
		float amplitudeFactor = std::powf(10.0f, gain / 20.0f); // Perhaps use a lookup table with linear interpolation for realtime mixing? (can calculate in realtime for extra accuracy when exporting)
		for (unsigned int i = 0; i < nChannels * nFrames; ++i)
			buffer[i] = amplitudeFactor * buffer[i];
	}

	void Mixer::applyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		if (nChannels != 2) return;

		// Sine-law panning
		float panning = (pan / 200.0f) + 0.5f;
		float pidiv2 = pi / 2.0f;
		float rightAmplitude = std::sinf(panning * pidiv2);
		float leftAmplitude = std::sinf((1.0f - panning) * pidiv2);

		// TODO: Vectorize this
		for (unsigned int frame = 0; frame < nFrames; ++frame)
		{
			buffer[frame] = leftAmplitude * buffer[frame]; // Left Channel
			buffer[nFrames + frame] = rightAmplitude * buffer[nFrames + frame]; // Right Channel
		}
	}

	void Mixer::processTrack(std::vector<float>& trackInputBuffer, TrackState::TrackIdentifier track, unsigned int nFrames, unsigned int sampleRate)
	{
		TrackState::Track trackInfo = audioEngine.trackState.getTrack(track);
		if (trackInfo.outputs.empty()) return;

		if (!trackBuffers.contains(track)) return;
		std::vector<float>& outputBuffer = trackBuffers[track].buffer;
		if (outputBuffer.empty()) return;

		for (unsigned int channel = 0; channel < (unsigned int)trackInfo.nChannels; ++channel)
		{
			// TODO: Apply effects
			for (int frame = 0; frame < nFrames; ++frame)
				outputBuffer[(channel * nFrames) + frame] = trackInputBuffer[(channel * nFrames) + frame];
		}

		// TODO: Support Surround Panning
		// Apply panning
		if (trackInfo.nChannels == TrackState::ChannelNumber::Stereo)
			applyStereoPanning(trackInfo.pan, outputBuffer, (unsigned int)trackInfo.nChannels, nFrames);

		// Apply gain
		applyGain(trackInfo.gain, outputBuffer, (unsigned int)trackInfo.nChannels, nFrames);

		// TODO: Vectorize this
		// Send out to buses
		for (unsigned int output = 0; output < trackInfo.outputs.size(); ++output)
		{
			TrackState::BusOutput busOutput = trackInfo.outputs[output];
			for (unsigned int channel = 0; channel < (unsigned int)trackInfo.nChannels; ++channel)
			{
				for (unsigned int outChannel : busOutput.inputChannelToOutputChannels[channel])
				{
					std::vector<float>& busOutputBuffer = busBuffers[busOutput.bus].buffer;
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						busOutputBuffer[(outChannel * nFrames) + frame] += outputBuffer[(channel * nFrames) + frame];
				}
			}
		}
	}

	void Mixer::processBus(TrackState::BusIdentifier bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate)
	{
		TrackState::Bus busInfo = audioEngine.trackState.getBus(bus);
		if (busInfo.outputs.empty() && busInfo.inputChannelToDeviceOutputChannels.empty()) return;

		if (!busBuffers.contains(bus)) return;
		std::vector<float>& outputBuffer = busBuffers[bus].buffer;
		if (outputBuffer.empty()) return;

		// TODO: Apply effects

		// TODO: Support Surround Panning
		// Apply panning
		if (busInfo.nChannels == TrackState::ChannelNumber::Stereo)
			applyStereoPanning(busInfo.pan, outputBuffer, (unsigned int)busInfo.nChannels, nFrames);

		// Apply gain
		applyGain(busInfo.gain, outputBuffer, (unsigned int)busInfo.nChannels, nFrames);

		// TODO: Vectorize this
		// Send out to buses
		for (unsigned int output = 0; output < busInfo.outputs.size(); ++output)
		{
			TrackState::BusOutput busOutput = busInfo.outputs[output];
			for (unsigned int channel = 0; channel < (unsigned int)busInfo.nChannels; ++channel)
			{
				for (unsigned int outChannel : busOutput.inputChannelToOutputChannels[channel])
				{
					std::vector<float>& busOutputBuffer = busBuffers[busOutput.bus].buffer;
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						busOutputBuffer[(outChannel * nFrames) + frame] += outputBuffer[(channel * nFrames) + frame];
				}
			}
		}
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
			const std::vector<TrackState::TrackIdentifier>& tracks = audioEngine.trackState.getAllTracks();
			const std::vector<TrackState::BusIdentifier>& buses = audioEngine.trackState.getAllBuses();

			// Currently we'll use noise for track inputs (for testing)
			std::vector<float> monoOutput;
			for (unsigned int frame = 0; frame < nFrames; ++frame)
				monoOutput.push_back(((float)rand() / RAND_MAX) + 1);

			// Zero out bus buffers
			for (const TrackState::BusIdentifier& bus : buses)
			{
				TrackState::Bus busInfo = audioEngine.trackState.getBus(bus);
				std::memset(busBuffers[bus].buffer.data(), 0, ((unsigned int)busInfo.nChannels * nFrames) * sizeof(float));
			}

			// Process Tracks
			for (const TrackState::TrackIdentifier& track : tracks) 
				processTrack(monoOutput, track, nFrames, sampleRate);
			// Process Buses
			for (const TrackState::BusIdentifier& bus : buses)
				processBus(bus, nFrames, nOutChannels, sampleRate);

			// Send out to output
			for (const TrackState::BusIdentifier& bus : buses)
			{
				TrackState::Bus busInfo = audioEngine.trackState.getBus(bus);
				// Send out to output device / buffer
				for (unsigned int channel = 0; channel < (unsigned int)busInfo.nChannels; ++channel)
				{
					for (unsigned int outChannel : busInfo.inputChannelToDeviceOutputChannels[channel])
					{
						if (outChannel >= nOutChannels) continue;
						for (unsigned int frame = 0; frame < nFrames; ++frame)
							outputBuffer[(outChannel * nFrames) + frame] += busBuffers[bus].buffer[(channel * nFrames) + frame];
					}
				}
			}
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
