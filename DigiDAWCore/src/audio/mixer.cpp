#include "digidaw/core/audio/mixer.h"

#include "digidaw/core/audio/engine.h"

#include "detail/simdhelper.h"

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
				trackBuffers[&track] = GetTrackBuffers(track, audioEngine.GetCurrentBufferSize(), (unsigned int)track.nChannels);
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
			trackBuffers[&track] = GetTrackBuffers(track, audioEngine.GetCurrentBufferSize(), (unsigned int)track.nChannels);
	}

	void Mixer::UpdateAllBusBuffers()
	{
		const std::vector<TrackState::Bus>& buses = audioEngine.trackState.GetAllBuses();

		busBuffers.clear();

		for (const TrackState::Bus& bus : buses)
			busBuffers[&bus] = MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)bus.nChannels);
	}

	Mixer::TrackBuffers Mixer::GetTrackBuffers(const TrackState::Track& track, unsigned int nFrames, unsigned int nChannels)
	{
		std::vector<std::vector<MixBuffer>> busOutputBuffers;

		// Each channel of this track has a mapping to the input channels of each bus it sends out to
		for (unsigned int output = 0; output < track.outputs.size(); ++output)
		{
			TrackState::BusOutput busOutput = track.outputs[output];
			std::vector<MixBuffer> channelBuffers;

			// Loop through each channel in the track and allocate the channel buffer for that output
			for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
				channelBuffers.push_back(MixBuffer(nFrames, busOutput.inputChannelToOutputChannels[channel].size()));

			busOutputBuffers.push_back(channelBuffers);
		}

		return TrackBuffers(MixBuffer(audioEngine.GetCurrentBufferSize(), (unsigned int)track.nChannels), busOutputBuffers);
	}

	void Mixer::UpdateCurrentTime(double time)
	{
		this->currentTime = time;
	}

	void Mixer::ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		// Perhaps use a lookup table for realtime mixing? (can calculate in realtime for extra accuracy when exporting)
		float amplitudeFactor = std::powf(10.0f, gain / 20.0f);
		Detail::SimdHelper::MulScalarBuffer(amplitudeFactor, buffer.data(), nChannels * nFrames, 0); // buffer = amplitudeFactor * buffer
	}

	void Mixer::ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		if (nChannels != 2) return;

		// Sine-law panning
		float panning = (pan / 200.0f) + 0.5f;
		const float pidiv2 = (const float)pi / 2.0f;
		float rightAmplitude = std::sinf(panning * pidiv2);
		float leftAmplitude = std::sinf((1.0f - panning) * pidiv2);

		// leftChannel = leftAmplitude * leftChannel
		// rightChannel = rightAmplitude * rightChannel
		Detail::SimdHelper::MulScalarBufferStereo(leftAmplitude, rightAmplitude, buffer.data(), nFrames, 0, nFrames);
	}

	void Mixer::ProcessTrack(std::vector<float>& trackInputBuffer, const TrackState::Track& track, unsigned int nFrames, unsigned int sampleRate)
	{
		if (track.outputs.empty()) return;

		if (!trackBuffers.contains(&track)) return;
		std::vector<float>& trackBuffer = trackBuffers[&track].mainTrackBuffer.buffer;
		if (trackBuffer.empty()) return;

		// Copy input buffer to track output buffer
		Detail::SimdHelper::CopyBuffer(trackInputBuffer.data(), trackBuffer.data(), 0, 0, (size_t)track.nChannels * nFrames);

		// TODO: Apply effects

		// Apply gain
		ApplyGain(track.gain, trackBuffer, (unsigned int)track.nChannels, nFrames);

		// TODO: Support Surround Panning
		// Apply panning
		if (track.nChannels == TrackState::ChannelNumber::Stereo)
			ApplyStereoPanning(track.pan, trackBuffer, (unsigned int)track.nChannels, nFrames);

		// Send out to buses by looping through all the bus outputs 
		// each channel of this track has a mapping to the input channels of each bus it sends out to
		for (unsigned int output = 0; output < track.outputs.size(); ++output)
		{
			TrackState::BusOutput busOutput = track.outputs[output];

			// Loop through each channel in the track to get it's corresponding outputs to the bus
			for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
			{
				// Copy the track buffer to this buffer
				// for expanding the amount of channels to the amount of channels we are sending to the bus
				// so that we can apply panning to Mono tracks.
				std::vector<float>& channelOutputBuffer = trackBuffers[&track].busOutputBuffers[output][channel].buffer;
				for (unsigned int outChannel = 0; outChannel < (unsigned int)busOutput.inputChannelToOutputChannels[channel].size(); ++outChannel)
					Detail::SimdHelper::CopyBuffer(trackBuffer.data(), channelOutputBuffer.data(), channel * nFrames, outChannel * nFrames, nFrames);

				// TODO: Support Surround Panning
				// Apply panning (for panning Mono tracks to Stereo buses)
				if (busOutput.inputChannelToOutputChannels[channel].size() == (size_t)TrackState::ChannelNumber::Stereo 
					&& track.nChannels == TrackState::ChannelNumber::Mono)
					ApplyStereoPanning(track.pan, channelOutputBuffer, busOutput.inputChannelToOutputChannels[channel].size(), nFrames);

				// Go through each output for this track channel (one track channel -> multiple bus channel mapping)
				unsigned int inChannel = 0;
				for (unsigned int outChannel : busOutput.inputChannelToOutputChannels[channel])
				{
					// Finally, pass the track output to the bus buffer (as input)
					std::vector<float>& busOutputBuffer = busBuffers[&busOutput.bus.get()].buffer;

					// busOutputBuffer[outChannel] += channelOutputBuffer[inChannel]
					Detail::SimdHelper::AddBuffer(
						channelOutputBuffer.data(),
						busOutputBuffer.data(), 
						inChannel * nFrames, outChannel * nFrames,
						nFrames);

					++inChannel;
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
		streamDeltaTime = currentTime - streamLastTime;

		const std::vector<TrackState::Track>& tracks = audioEngine.trackState.GetAllTracks();
		const std::vector<TrackState::Bus>& buses = audioEngine.trackState.GetAllBuses();

		if (!doTestTone)
		{
			// Zero out bus buffers
			for (const TrackState::Bus& bus : buses)
				Detail::SimdHelper::SetBuffer(busBuffers[&bus].buffer.data(), 0.0f, (size_t)bus.nChannels * nFrames, 0);

			// Process Tracks
			for (const TrackState::Track& track : tracks)
			{
				// Currently we'll use noise for track inputs (for testing)
				std::vector<float> trackInput((size_t)track.nChannels * nFrames);
				for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
					for (unsigned int frame = 0; frame < nFrames; ++frame)
						trackInput[(channel * nFrames) + frame] = ((float)rand() / RAND_MAX) + 1.0f;
						//trackInput[(channel * nFrames) + frame] = 0.0f;
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
						if (outChannel >= nOutChannels) continue;

						// TODO: Mono Bus Panning

						// outputBuffer[outChannel] += busBuffers[bus].buffer[channel]
						Detail::SimdHelper::AddBuffer(
							busBuffers[&bus].buffer.data(), 
							outputBuffer, 
							channel * nFrames, outChannel * nFrames, 
							nFrames);
					}
				}
			}
		}
		else
		{
			std::vector<float> monoOutput(nFrames);
			for (unsigned int frame = 0; frame < nFrames; ++frame)
			{
				double sampleTime = (time + ((double)frame / (double)sampleRate)) - testToneStartTime;
				float amplitude = (float)((0.10 * (std::clamp(1.0 - sampleTime, 0.0, 1.0))) * ((std::sinf(2 * pi * 440.0 * sampleTime) * 0.5f) + 0.5f));
				monoOutput[frame] = amplitude;
			}

			for (unsigned int channel = 0; channel < nOutChannels; ++channel)
				std::memcpy(&outputBuffer[channel * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
		}

		// Update Mixable Info on an interval (no need to do it every buffer frame)
		updateCounter += streamDeltaTime;
		if (updateCounter >= updateInterval)
		{
			updateCounter = 0.0;

			std::vector<float> tempBuffer;

			for (const TrackState::Bus& bus : buses)
			{
				Detail::SimdHelper::GetBufferAverageMultiChannel(
					busBuffers[&bus].buffer.data(),
					nFrames, (size_t)bus.nChannels,
					tempBuffer);
				mixableInfo[&bus].channels.resize((size_t)bus.nChannels);
				for (unsigned int channel = 0; channel < (unsigned int)bus.nChannels; ++channel)
					mixableInfo[&bus].channels[channel].averageAmplitude = tempBuffer[channel];
			}

			for (const TrackState::Track& track : tracks)
			{
				Detail::SimdHelper::GetBufferAverageMultiChannel(
					trackBuffers[&track].mainTrackBuffer.buffer.data(),
					nFrames, (size_t)track.nChannels,
					tempBuffer);
				mixableInfo[&track].channels.resize((size_t)track.nChannels);
				for (unsigned int channel = 0; channel < (unsigned int)track.nChannels; ++channel)
					mixableInfo[&track].channels[channel].averageAmplitude = tempBuffer[channel];
			}

			Detail::SimdHelper::GetBufferAverageMultiChannel(
				outputBuffer, 
				nFrames, nOutChannels, 
				tempBuffer);
			outputChannels.resize(nOutChannels);
			for (unsigned int channel = 0; channel < nOutChannels; ++channel)
				outputChannels[channel].averageAmplitude = tempBuffer[channel];
		}

		streamLastTime = currentTime;
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
