#include "digidaw/core/audio/mixer.h"

#include "digidaw/core/audio/engine.h"

namespace DigiDAW::Core::Audio
{
	Mixer::Mixer(Engine& audioEngine) 
		: audioEngine(audioEngine)
	{
		this->doTestTone = false;
		this->testToneStartTime = 0.0;
		this->currentTime = 0.0;

		audioEngine.trackState.addTrackCallbacks.push_back(
			[&](TrackState::Track& track) 
			{
				trackBuffers[&track] = MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)track.nChannels);
			});
		audioEngine.trackState.removeTrackCallbacks.push_back(
			[&](TrackState::Track& track) 
			{
				trackBuffers.erase(&track);
			});

		audioEngine.trackState.addBusCallbacks.push_back(
			[&](TrackState::Bus bus) 
			{
				busBuffers[&bus] = MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)bus.nChannels);
			});
		audioEngine.trackState.removeBusCallbacks.push_back(
			[&](TrackState::Bus bus) 
			{
				busBuffers.erase(&bus);
			});
	}

	void Mixer::UpdateAllTrackBuffers()
	{
		const std::vector<TrackState::Track>& tracks = audioEngine.trackState.GetAllTracks();

		trackBuffers.clear();

		for (const TrackState::Track& track : tracks)
			trackBuffers[&track] = MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)track.nChannels);
	}

	void Mixer::UpdateAllBusBuffers()
	{
		const std::vector<TrackState::Bus>& buses = audioEngine.trackState.GetAllBuses();

		busBuffers.clear();

		for (const TrackState::Bus& bus : buses)
			busBuffers[&bus] = MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)bus.nChannels);
	}

	void Mixer::UpdateCurrentTime(double time)
	{
		this->currentTime = time;
	}

	void Mixer::ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		// TODO: Vectorize this
		float amplitudeFactor = std::powf(10.0f, gain / 20.0f); // Perhaps use a lookup table for realtime mixing? (can calculate in realtime for extra accuracy when exporting)
		for (unsigned int i = 0; i < nChannels * nFrames; ++i)
			buffer[i] = amplitudeFactor * buffer[i];
	}

	void Mixer::ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		if (nChannels != 2) return;

		// Sine-law panning
		float panning = (pan / 200.0f) + 0.5f;
		const float pidiv2 = (float)pi / 2.0f;
		float rightAmplitude = std::sinf(panning * pidiv2);
		float leftAmplitude = std::sinf((1.0f - panning) * pidiv2);

		// TODO: Vectorize this
		for (unsigned int frame = 0; frame < nFrames; ++frame)
		{
			buffer[frame] = leftAmplitude * buffer[frame]; // Left Channel
			buffer[nFrames + frame] = rightAmplitude * buffer[nFrames + frame]; // Right Channel
		}
	}

	void Mixer::ProcessTrack(std::vector<float>& trackInputBuffer, const TrackState::Track& track, unsigned int nFrames, unsigned int sampleRate)
	{
		if (track.outputs.empty()) return;

		if (!trackBuffers.contains(&track)) return;
		std::vector<float>& trackBuffer = trackBuffers[&track].buffer;
		if (trackBuffer.empty()) return;

		// Copy input buffer to track output buffer
		for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
		{
			// TODO: Apply effects
			for (unsigned int frame = 0; frame < nFrames; ++frame)
				trackBuffer[(channel * nFrames) + frame] = trackInputBuffer[(channel * nFrames) + frame];
		}

		// Apply gain
		ApplyGain(track.gain, trackBuffer, (unsigned int)track.nChannels, nFrames);

		// TODO: Support Surround Panning
		// Apply panning
		if (track.nChannels == TrackState::ChannelNumber::Stereo)
			ApplyStereoPanning(track.pan, trackBuffer, (unsigned int)track.nChannels, nFrames);

		// TODO: Vectorize this
		// Send out to buses by looping through all the bus outputs 
		// each channel of this track has a mapping to the input channels of each bus it sends out to
		for (unsigned int output = 0; output < track.outputs.size(); ++output)
		{
			TrackState::BusOutput busOutput = track.outputs[output];

			// Loop through each channel in the track to get it's corresponding outputs to the bus
			for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
			{
				// Create a new buffer to copy the track buffer to for this specific bus channel output 
				// (it's probably a good idea to pre-allocate this)
				std::vector<float> channelOutputBuffer(nFrames * busOutput.inputChannelToOutputChannels[channel].size());
				for (unsigned int outChannel = 0; outChannel < (unsigned int)busOutput.inputChannelToOutputChannels[channel].size(); ++outChannel)
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						channelOutputBuffer[(outChannel * nFrames) + frame] = trackBuffer[(channel * nFrames) + frame];

				// TODO: Support Surround Panning
				// Apply panning (for panning mono tracks to stereo buses)
				if (busOutput.inputChannelToOutputChannels[channel].size() == (size_t)TrackState::ChannelNumber::Stereo
					&& track.nChannels == TrackState::ChannelNumber::Mono)
					ApplyStereoPanning(track.pan, channelOutputBuffer, busOutput.inputChannelToOutputChannels[channel].size(), nFrames);

				// Go through each output for this track channel (one track channel -> multiple bus channel mapping)
				unsigned int channelIndex = 0;
				for (unsigned int outChannel : busOutput.inputChannelToOutputChannels[channel])
				{
					// Finally, pass the track output to the bus buffer (as input)
					std::vector<float>& busOutputBuffer = busBuffers[&busOutput.bus.get()].buffer;
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						busOutputBuffer[(outChannel * nFrames) + frame] += channelOutputBuffer[(channelIndex * nFrames) + frame];

					++channelIndex;
				}
			}
		}
	}

	void Mixer::ProcessBus(const TrackState::Bus& bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate)
	{
		if (bus.busChannelToDeviceOutputChannels.empty()) return;

		if (!busBuffers.contains(&bus)) return;
		std::vector<float>& outputBuffer = busBuffers[&bus].buffer;
		if (outputBuffer.empty()) return;

		// TODO: Apply effects

		// TODO: Support Surround Panning
		// Apply panning
		if (bus.nChannels == TrackState::ChannelNumber::Stereo)
			ApplyStereoPanning(bus.pan, outputBuffer, (unsigned int)bus.nChannels, nFrames);

		// Apply gain
		ApplyGain(bus.gain, outputBuffer, (unsigned int)bus.nChannels, nFrames);
	}

	void Mixer::Mix(
		float* outputBuffer,
		float* inputBuffer, 
		double time, 
		unsigned int nFrames,
		unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate)
	{
		currentTime = time;

		if (!doTestTone)
		{
			const std::vector<TrackState::Track>& tracks = audioEngine.trackState.GetAllTracks();
			const std::vector<TrackState::Bus>& buses = audioEngine.trackState.GetAllBuses();

			// Zero out bus buffers
			for (const TrackState::Bus& bus : buses)
				std::memset(busBuffers[&bus].buffer.data(), 0, ((unsigned int)bus.nChannels * nFrames) * sizeof(float));

			// Process Tracks
			for (const TrackState::Track& track : tracks)
			{
				// Currently we'll use noise for track inputs (for testing)
				std::vector<float> trackInput((size_t)track.nChannels * nFrames);
				for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						trackInput[(channel * nFrames) + frame] = ((float)rand() / RAND_MAX) + 1.0f;
				ProcessTrack(trackInput, track, nFrames, sampleRate);
			}

			// Process Buses
			for (const TrackState::Bus& bus : buses)
				ProcessBus(bus, nFrames, nOutChannels, sampleRate);

			// Send out to output
			for (const TrackState::Bus& bus : buses)
			{
				// Send out to output device / buffer
				for (unsigned int channel = 0; channel < (unsigned int)bus.nChannels; ++channel)
				{
					for (unsigned int outChannel : bus.busChannelToDeviceOutputChannels[channel])
					{
						// TODO: Vectorize this
						if (outChannel >= nOutChannels) continue;

						// TODO: Mono Bus Panning
						for (unsigned int frame = 0; frame < nFrames; ++frame)
							outputBuffer[(outChannel * nFrames) + frame] += busBuffers[&bus].buffer[(channel * nFrames) + frame];
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

			for (unsigned int channel = 0; channel < nOutChannels; ++channel)
				std::memcpy(&outputBuffer[channel * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
		}
	}

	void Mixer::StartTestTone()
	{
		testToneStartTime = currentTime;
		doTestTone = true;
	}

	void Mixer::EndTestTone()
	{
		testToneStartTime = 0.0;
		doTestTone = false;
	}
}
