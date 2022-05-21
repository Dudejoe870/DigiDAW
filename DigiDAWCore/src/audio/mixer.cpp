#include "digidaw/core/audio/mixer.h"
#include "digidaw/core/audio/engine.h"

#include "detail/simdhelper.h"

namespace DigiDAW::Core::Audio
{
	Mixer::TrackInfo::TrackInfo(Engine& audioEngine, const std::shared_ptr<TrackState::Track>& track, unsigned int nFrames)
	{
		this->mainTrackBuffer = MixBuffer(audioEngine.GetCurrentBufferSize(), static_cast<unsigned int>(track->nChannels));

		// Each channel of this track has a mapping to the input channels of each bus it sends out to
		for (unsigned int output = 0; output < track->outputs.size(); ++output)
		{
			const TrackState::BusOutput& busOutput = track->outputs[output];
			std::vector<MixBuffer> channelBuffers;

			// Loop through each channel in the track and allocate the channel buffer for that output
			for (unsigned int channel = 0; channel < static_cast<unsigned int>(track->nChannels); ++channel)
				channelBuffers.push_back(MixBuffer(nFrames, busOutput.inputChannelToOutputChannels[channel].size()));

			busOutputBuffers.push_back(channelBuffers);
		}
	}

	Mixer::Mixer(Engine& audioEngine) 
		: audioEngine(audioEngine)
	{
		this->doTestTone = false;
		this->testToneStartTime = 0.0;
		this->currentTime = 0.0;

		audioEngine.trackState.addTrackCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Track> track) 
			{
				trackInfo[track.get()] = TrackInfo(audioEngine, track, audioEngine.GetCurrentBufferSize());
			});
		audioEngine.trackState.removeTrackCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Track> track)
			{
				trackInfo.erase(track.get());
				mixableInfo.erase(track.get());
			});

		audioEngine.trackState.addBusCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Bus> bus)
			{
				busInfo[bus.get()] = BusInfo(bus, audioEngine.GetCurrentBufferSize());
			});
		audioEngine.trackState.removeBusCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Bus> bus)
			{
				busInfo.erase(bus.get());
				mixableInfo.erase(bus.get());
			});

		mixerThread = std::jthread(
			[&]()
			{
				const std::vector<std::shared_ptr<TrackState::Track>>& tracks = audioEngine.trackState.GetAllTracks();
				const std::vector<std::shared_ptr<TrackState::Bus>>& buses = audioEngine.trackState.GetAllBuses();

				std::vector<float> rmsBuffer;
				std::vector<float> peakBuffer;

				auto currentTime = std::chrono::high_resolution_clock::now();
				auto lastTime = currentTime;

				while (running)
				{
					currentTime = std::chrono::high_resolution_clock::now();
					unsigned long long deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::duration<double>(currentTime - lastTime)).count();

					if (!tracks.empty() || !buses.empty())
					{
						for (const std::shared_ptr<TrackState::Bus>& bus : buses)
						{
							if (!mixableInfo[bus.get()].lookbackBuffers.empty())
							{
								std::lock_guard<std::mutex> lock(mixableInfo[bus.get()].lookbackBufferMutex);

								Detail::SimdHelper::GetBufferRMSAndPeakMultiChannel(
									mixableInfo[bus.get()].lookbackBuffers,
									mixableInfo[bus.get()].lookbackBuffers[0].size(),
									rmsBuffer,
									peakBuffer);
								mixableInfo[bus.get()].channels.resize(static_cast<std::size_t>(bus->nChannels));

								for (unsigned int channel = 0; channel < static_cast<unsigned int>(bus->nChannels); ++channel)
								{
									LerpMeter(mixableInfo[bus.get()].channels[channel].rms, rmsBuffer[channel],
										(float)deltaTime,
										(float)meterRMSRiseTimeMS, (float)meterRMSFallTimeMS,
										minimumDecibelLevel);
									LerpMeter(mixableInfo[bus.get()].channels[channel].peak, peakBuffer[channel],
										(float)deltaTime,
										(float)meterPeakRiseTimeMS, (float)meterPeakFallTimeMS,
										minimumDecibelLevel);

									bool& clip = mixableInfo[bus.get()].channels[channel].clip;
									if (!clip)
										clip = mixableInfo[bus.get()].channels[channel].peak >= 0.0f;
								}
							}
						}

						for (const std::shared_ptr<TrackState::Track>& track : tracks)
						{
							if (!mixableInfo[track.get()].lookbackBuffers.empty())
							{
								std::lock_guard<std::mutex> lock(mixableInfo[track.get()].lookbackBufferMutex);

								Detail::SimdHelper::GetBufferRMSAndPeakMultiChannel(
									mixableInfo[track.get()].lookbackBuffers,
									mixableInfo[track.get()].lookbackBuffers[0].size(),
									rmsBuffer,
									peakBuffer);
								mixableInfo[track.get()].channels.resize(static_cast<std::size_t>(track->nChannels));

								for (unsigned int channel = 0; channel < static_cast<unsigned int>(track->nChannels); ++channel)
								{
									LerpMeter(mixableInfo[track.get()].channels[channel].rms, rmsBuffer[channel],
										(float)deltaTime,
										(float)meterRMSRiseTimeMS, (float)meterRMSFallTimeMS,
										minimumDecibelLevel);
									LerpMeter(mixableInfo[track.get()].channels[channel].peak, peakBuffer[channel],
										(float)deltaTime,
										(float)meterPeakRiseTimeMS, (float)meterPeakFallTimeMS,
										minimumDecibelLevel);

									bool& clip = mixableInfo[track.get()].channels[channel].clip;
									if (!clip)
										clip = mixableInfo[track.get()].channels[channel].peak >= 0.0f;
								}
							}
						}

						if (!outputInfo.lookbackBuffers.empty())
						{
							std::lock_guard<std::mutex> lock(outputInfo.lookbackBufferMutex);

							Detail::SimdHelper::GetBufferRMSAndPeakMultiChannel(
								outputInfo.lookbackBuffers,
								outputInfo.lookbackBuffers[0].size(),
								rmsBuffer,
								peakBuffer);
							outputInfo.channels.resize(nOutChannels);

							for (unsigned int channel = 0; channel < outputInfo.channels.size(); ++channel)
							{
								LerpMeter(outputInfo.channels[channel].rms, rmsBuffer[channel], 
									(float)deltaTime,
									(float)meterRMSRiseTimeMS, (float)meterRMSFallTimeMS, 
									minimumDecibelLevel);
								LerpMeter(outputInfo.channels[channel].peak, peakBuffer[channel],
									(float)deltaTime,
									(float)meterPeakRiseTimeMS, (float)meterPeakFallTimeMS,
									minimumDecibelLevel);

								bool& clip = outputInfo.channels[channel].clip;
								if (!clip)
									clip = outputInfo.channels[channel].peak >= 0.0f;
							}
						}
					}

					lastTime = currentTime;
					std::this_thread::sleep_for(std::chrono::milliseconds(meterUpdateIntervalMS)); // Doesn't need to be accurate
				}
			});
	}

	Mixer::~Mixer()
	{
		running = false;
	}

	void Mixer::UpdateAllTrackBuffers()
	{
		const std::vector<std::shared_ptr<TrackState::Track>>& tracks = audioEngine.trackState.GetAllTracks();

		trackInfo.clear();

		for (const std::shared_ptr<TrackState::Track>& track : tracks)
			trackInfo[track.get()] = TrackInfo(audioEngine, track, audioEngine.GetCurrentBufferSize());
	}

	void Mixer::UpdateAllBusBuffers()
	{
		const std::vector<std::shared_ptr<TrackState::Bus>>& buses = audioEngine.trackState.GetAllBuses();

		busInfo.clear();

		for (const std::shared_ptr<TrackState::Bus>& bus : buses)
			busInfo[bus.get()] = BusInfo(bus, audioEngine.GetCurrentBufferSize());
	}

	inline void Mixer::ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		// Perhaps use a lookup table for realtime mixing? (can calculate in realtime for extra accuracy when exporting)
		float amplitudeFactor = std::powf(10.0f, gain / 20.0f);
		Detail::SimdHelper::MulScalarBuffer(amplitudeFactor, buffer.data(), nChannels * nFrames, 0); // buffer = amplitudeFactor * buffer
	}

	inline void Mixer::ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames)
	{
		if (nChannels != 2) return;

		// Sine-law panning
		float panning = (pan / 200.0f) + 0.5f;
		const float pidiv2 = pi<float> / 2.0f;
		float rightAmplitude = std::sinf(panning * pidiv2);
		float leftAmplitude = std::sinf((1.0f - panning) * pidiv2);

		// leftChannel = leftAmplitude * leftChannel
		// rightChannel = rightAmplitude * rightChannel
		Detail::SimdHelper::MulScalarBufferStereo(leftAmplitude, rightAmplitude, buffer.data(), nFrames, 0, nFrames);
	}

	inline void Mixer::ProcessTrack(
		std::vector<float>& trackInputBuffer, const std::shared_ptr<TrackState::Track>& track,
		unsigned int nFrames, unsigned int sampleRate)
	{
		if (track->outputs.empty()) return;

		if (!trackInfo.contains(track.get())) return;
		std::vector<float>& trackBuffer = trackInfo[track.get()].mainTrackBuffer.buffer;
		if (trackBuffer.empty()) return;

		// Copy input buffer to track output buffer
		Detail::SimdHelper::CopyBuffer(trackInputBuffer.data(), trackBuffer.data(), 0, 0, 
			static_cast<std::size_t>(track->nChannels) * nFrames);

		// TODO: Apply effects

		// Apply gain
		ApplyGain(track->gain, trackBuffer, static_cast<unsigned int>(track->nChannels), nFrames);

		// TODO: Support Surround Panning
		// Apply panning
		if (track->nChannels == TrackState::ChannelNumber::Stereo)
			ApplyStereoPanning(track->pan, trackBuffer, static_cast<unsigned int>(track->nChannels), nFrames);

		// Add final output to the lookback buffer
		AddToLookback(trackBuffer.data(), 
			mixableInfo[track.get()].lookbackBuffers,
			mixableInfo[track.get()].lookbackBufferMutex,
			nFrames, static_cast<unsigned int>(track->nChannels), sampleRate);
	}

	inline void Mixer::ProcessBus(
		const std::shared_ptr<TrackState::Bus>& bus,
		unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate)
	{
		if (bus->busChannelToDeviceOutputChannels.empty()) return;

		if (!busInfo.contains(bus.get())) return;
		std::vector<float>& busBuffer = busInfo[bus.get()].mainBusBuffer.buffer;
		if (busBuffer.empty()) return;

		// TODO: Apply effects

		// TODO: Support Surround Panning
		// Apply panning
		if (bus->nChannels == TrackState::ChannelNumber::Stereo)
			ApplyStereoPanning(bus->pan, busBuffer, static_cast<unsigned int>(bus->nChannels), nFrames);

		// Apply gain
		ApplyGain(bus->gain, busBuffer, static_cast<unsigned int>(bus->nChannels), nFrames);

		// Add final output to the lookback buffer
		AddToLookback(busBuffer.data(), 
			mixableInfo[bus.get()].lookbackBuffers,
			mixableInfo[bus.get()].lookbackBufferMutex,
			nFrames, static_cast<unsigned int>(bus->nChannels), sampleRate);
	}

	void Mixer::ResetClippingIndicators()
	{
		for (auto& pair : mixableInfo)
		{
			MixableInfo& info = pair.second;
			for (ChannelInfo& channel : info.channels)
				channel.clip = false;
		}

		for (ChannelInfo& channel : outputInfo.channels)
			channel.clip = false;
	}

	void Mixer::Mix(
		float* outputBuffer,
		float* inputBuffer, 
		double time, 
		unsigned int nFrames,
		unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate)
	{
		this->nOutChannels = nOutChannels;

		currentTime = time;

		const std::vector<std::shared_ptr<TrackState::Track>>& tracks = audioEngine.trackState.GetAllTracks();
		const std::vector<std::shared_ptr<TrackState::Bus>>& buses = audioEngine.trackState.GetAllBuses();

		if (!doTestTone)
		{
			// Zero out bus buffers
			for (const std::shared_ptr<TrackState::Bus>& bus : buses)
				Detail::SimdHelper::SetBuffer(busInfo[bus.get()].mainBusBuffer.buffer.data(), 0.0f,
					static_cast<std::size_t>(bus->nChannels) * nFrames, 0);

			// Process Tracks (one thread per track)
			trackThreads.Resize(tracks.size());
			for (const std::shared_ptr<TrackState::Track>& track : tracks)
			{
				// Track Sends will probably complicate this more, 
				// however effects need to be worked on first before 
				// being able to implement that. However the basic idea 
				// is that you'd just wait on the tracks that need to be 
				// sent to certain effects to finish their "processAsync" std::futures, 
				// and then use the final track buffer to input into the effect.
				// Of course this'd be slower than doing it completely parallel, 
				// but it's necessary since the track being sent to would depend 
				// on the other track to finish processing.
				trackInfo[track.get()].processAsync = trackThreads.Queue(
					[&]()
					{
						//thread_local static std::random_device rd; // For debugging
						//thread_local static std::mt19937 rng(rd()); // For debugging
						//thread_local std::uniform_real_distribution<float> urd(0.0f, 1.0f); // For debugging

						// Currently we'll use silence for track inputs
						std::vector<float> trackInput(static_cast<std::size_t>(track->nChannels) * nFrames);
						for (unsigned int channel = 0; channel < static_cast<unsigned int>(track->nChannels); ++channel)
						{
							for (unsigned int frame = 0; frame < nFrames; ++frame)
							{
								//trackInput[(channel * nFrames) + frame] = urd(rng); // For debugging
								
								//double sampleTime = (time + (static_cast<double>(frame) / static_cast<double>(sampleRate))); // For debugging
								//trackInput[(channel * nFrames) + frame] = (std::cosf(2.0f * pi<float> * 440.0f * sampleTime) * 0.5f) + 0.5f; // For debugging

								trackInput[(channel * nFrames) + frame] = 0.0f;
							}
						}
						ProcessTrack(trackInput, track, nFrames, sampleRate);
					});
			}

			// Send tracks to bus outputs
			for (const std::shared_ptr<TrackState::Track>& track : tracks)
			{
				// Wait for this tracks thread to finish.
				trackInfo[track.get()].processAsync.wait();

				// Send out to buses by looping through all the bus outputs 
				// each channel of this track has a mapping to the input channels of each bus it sends out to
				for (unsigned int output = 0; output < track->outputs.size(); ++output)
				{
					TrackState::BusOutput busOutput = track->outputs[output];

					// Loop through each channel in the track to get it's corresponding outputs to the bus
					for (unsigned int channel = 0; channel < static_cast<unsigned int>(track->nChannels); ++channel)
					{
						// Copy the track buffer to this buffer
						// for expanding the amount of channels to the amount of channels we are sending to the bus
						// so that we can apply panning to Mono tracks.
						std::vector<float>& channelOutputBuffer = trackInfo[track.get()].busOutputBuffers[output][channel].buffer;
						for (unsigned int outChannel = 0; outChannel < static_cast<unsigned int>(
								busOutput.inputChannelToOutputChannels[channel].size()); ++outChannel)
							Detail::SimdHelper::CopyBuffer(trackInfo[track.get()].mainTrackBuffer.buffer.data(), channelOutputBuffer.data(),
								channel * nFrames, outChannel * nFrames, nFrames);

						// TODO: Support Surround Panning
						// Apply panning (for panning Mono tracks to Stereo buses)
						if (busOutput.inputChannelToOutputChannels[channel].size() == static_cast<std::size_t>(TrackState::ChannelNumber::Stereo)
							&& track->nChannels == TrackState::ChannelNumber::Mono)
							ApplyStereoPanning(track->pan, channelOutputBuffer, busOutput.inputChannelToOutputChannels[channel].size(), nFrames);

						// Go through each output for this track channel (one track channel -> multiple bus channel mapping)
						unsigned int inChannel = 0;
						for (unsigned int outChannel : busOutput.inputChannelToOutputChannels[channel])
						{
							// Finally, pass the track output to the bus buffer (as input)
							std::vector<float>& busOutputBuffer = busInfo[busOutput.bus.get()].mainBusBuffer.buffer;

							// busOutputBuffer[outChannel] += channelOutputBuffer[inChannel]
							Detail::SimdHelper::AccumulateBuffer(
								channelOutputBuffer.data(),
								busOutputBuffer.data(),
								inChannel * nFrames, outChannel * nFrames,
								nFrames);

							++inChannel;
						}
					}
				}
			}

			// Process Buses
			for (const std::shared_ptr<TrackState::Bus>& bus : buses)
				ProcessBus(bus, nFrames, nOutChannels, sampleRate);

			// Send buses to output
			for (const std::shared_ptr<TrackState::Bus>& bus : buses)
			{
				// Send out to output device / buffer
				for (unsigned int channel = 0; channel < static_cast<unsigned int>(bus->nChannels); ++channel)
				{
					for (unsigned int outChannel : bus->busChannelToDeviceOutputChannels[channel])
					{
						if (outChannel >= nOutChannels) continue;

						// TODO: Mono Bus Panning

						// outputBuffer[outChannel] += busBuffers[bus].buffer[channel]
						Detail::SimdHelper::AccumulateBuffer(
							busInfo[bus.get()].mainBusBuffer.buffer.data(),
							outputBuffer, 
							channel * nFrames, outChannel * nFrames, 
							nFrames);
					}
				}
			}
		}
		else
		{
			std::vector<float> testToneBuffer(nFrames);
			for (unsigned int frame = 0; frame < nFrames; ++frame)
			{
				double sampleTime = (time + (static_cast<double>(frame) / static_cast<double>(sampleRate))) - testToneStartTime;
				float amplitude = static_cast<float>(0.50 * 
					((std::clamp(1.0 - sampleTime, 0.0, 1.0)) * // Fade Out
					(std::clamp(sampleTime, 0.0, 1.0))) * // Fade In
					((std::cosf(2 * pi<double> * 440.0 * sampleTime) * 0.5f) + 0.5f));
				testToneBuffer[frame] = amplitude;
			}

			for (unsigned int channel = 0; channel < nOutChannels; ++channel)
				Detail::SimdHelper::CopyBuffer(testToneBuffer.data(), outputBuffer, 0, channel * nFrames, testToneBuffer.size());
		}

		AddToLookback(outputBuffer, 
			outputInfo.lookbackBuffers,
			outputInfo.lookbackBufferMutex, 
			nFrames, nOutChannels, sampleRate);
	}

	inline void Mixer::AddToLookback(
		float* src, std::vector<std::vector<float>>& dst, 
		std::mutex& mutex, 
		unsigned nFrames, unsigned int nChannels, unsigned int sampleRate)
	{
		std::lock_guard<std::mutex> lock(mutex);

		std::size_t amountOfSamples = static_cast<std::size_t>(
			(static_cast<float>(lookbackBufferIntervalMS) / 1000.0f) * static_cast<float>(sampleRate));

		dst.resize(nChannels);
		for (unsigned int channel = 0; channel < nChannels; ++channel)
		{
			std::vector<float>& buffer = dst[channel];
			buffer.reserve(amountOfSamples);
			std::size_t offset = buffer.size();
			if (buffer.size() + nFrames >= amountOfSamples)
			{
				// TODO: OPTIMIZE THIS PLEASE
				// This is really not very optimal
				// currently I'm not going to concern myself
				// with the performance implications of this
				// but if performance is bad, this is probably one 
				// of the first places I'd look, as this is called 
				// every single audio frame, and we're basically using a 
				// vector as a circular buffer. And while it'd be nice to 
				// use a more optimal data structure like an actual circular 
				// buffer, in this case the size can actually change, 
				// and we need the memory to be contiguous.
				// One idea is instead of using the erase function,
				// we could write a SIMD vector shift function to shift 
				// everything over by nFrames, making room for the new data.
				// However there could still be a more optimal way.
				// (remember we need to get the peaks and averages, 
				// but VSTs will need this to be in the correct order too)

				buffer.resize(amountOfSamples);
				offset -= nFrames;
				buffer.erase(buffer.begin(), buffer.begin() + nFrames); // FIX ME: There's a bug that crashes here, see Issue #1
			}
			else buffer.resize(buffer.size() + nFrames);

			Detail::SimdHelper::CopyBuffer(src, buffer.data(), channel * nFrames, offset, nFrames);
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
