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
			[&](std::shared_ptr<TrackState::Track> track) 
			{
				std::lock_guard<std::mutex> lock(audioProcessingMutex); // Make sure we aren't currently using the data.
				trackInfo[track.get()] = TrackInfo(track, audioEngine.GetCurrentBufferSize());
			});
		audioEngine.trackState.removeTrackCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Track> track)
			{
				std::lock_guard<std::mutex> lock(audioProcessingMutex);
				trackInfo.erase(track.get());
				mixableInfo.erase(track.get());
			});

		audioEngine.trackState.addBusCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Bus> bus)
			{
				std::lock_guard<std::mutex> lock(audioProcessingMutex);
				busInfo[bus.get()] = BusInfo(bus, audioEngine.GetCurrentBufferSize());
			});
		audioEngine.trackState.removeBusCallbacks.push_back(
			[&](std::shared_ptr<TrackState::Bus> bus)
			{
				std::lock_guard<std::mutex> lock(audioProcessingMutex);
				busInfo.erase(bus.get());
				mixableInfo.erase(bus.get());
			});

		mixerThread = std::jthread(
			[&]()
			{
				std::vector<float> rmsBuffer;
				std::vector<float> peakBuffer;

				auto currentTime = std::chrono::high_resolution_clock::now();
				auto lastTime = currentTime;

				while (running)
				{
					// Copy tracks and buses to new vectors just incase any changes are made on a separate thread during processing.
					const std::vector<std::shared_ptr<TrackState::Track>> tracks = audioEngine.trackState.ThreadedCopyAllTracks();
					const std::vector<std::shared_ptr<TrackState::Bus>> buses = audioEngine.trackState.ThreadedCopyAllBuses();

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
			trackInfo[track.get()] = TrackInfo(track, audioEngine.GetCurrentBufferSize());
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

		// Note: These two loops do essentially the same thing
		// just on different structs. We could use polymorphism, but then you'd probably have 
		// to do a reinterpret_cast and get a little funky with it. And I think it's fairly easy 
		// to understand with this code, just a little repetitive.

		// Process all the track inputs
		for (unsigned int input = 0; input < bus->trackInputs.size(); ++input)
		{
			const TrackState::TrackInput& trackInput = bus->trackInputs[input];
			mixableInfo[trackInput.track.get()].processAsync.wait(); // Wait for the specified track to finish processing.

			// Loop through each channel in the track to get its corresponding outputs to the bus
			for (unsigned int channel = 0; channel < static_cast<unsigned int>(trackInput.track->nChannels); ++channel)
			{
				if (trackInput.trackToBusMap.mapping[channel].empty()) continue;

				// Copy the track buffer to this buffer
				// for expanding the amount of channels that the track has to the amount of channels of the bus
				// so that we can apply panning to Mono tracks.
				std::vector<float>& channelInputBuffer = busInfo[bus.get()].trackInputBuffers[input][channel].buffer;
				for (unsigned int inChannel = 0; inChannel < static_cast<unsigned int>(
					trackInput.trackToBusMap.mapping[channel].size()); ++inChannel)
					Detail::SimdHelper::CopyBuffer(
						trackInfo[trackInput.track.get()].mainTrackBuffer.buffer.data(),
						channelInputBuffer.data(),
						channel * nFrames, inChannel * nFrames, nFrames);

				// TODO: Support Surround Panning
				// Apply panning (for panning Mono tracks to Stereo buses)
				if (trackInput.trackToBusMap.mapping[channel].size() == static_cast<std::size_t>(TrackState::ChannelNumber::Stereo)
					&& trackInput.track->nChannels == TrackState::ChannelNumber::Mono)
					ApplyStereoPanning(trackInput.track->pan, channelInputBuffer,
						trackInput.trackToBusMap.mapping[channel].size(),
						nFrames);

				// Go through each output for this track channel (one track channel -> multiple bus channel mapping)
				unsigned int trackChannel = 0;
				for (unsigned int busChannel : trackInput.trackToBusMap.mapping[channel])
				{
					// Finally, pass the track output to the bus buffer as the input
					std::vector<float>& busOutputBuffer = busInfo[bus.get()].mainBusBuffer.buffer;

					// busOutputBuffer[busChannel] += channelOutputBuffer[trackChannel]
					Detail::SimdHelper::AccumulateBuffer(
						channelInputBuffer.data(),
						busOutputBuffer.data(),
						trackChannel * nFrames, busChannel * nFrames,
						nFrames);

					++trackChannel;
				}
			}
		}

		// Process all Bus inputs
		for (unsigned int input = 0; input < bus->busInputs.size(); ++input)
		{
			const TrackState::BusInput& busInput = bus->busInputs[input];
			mixableInfo[busInput.bus.get()].processAsync.wait(); // Wait for the specified bus to finish processing.

			// Loop through each channel in the source bus to get its corresponding outputs to this bus
			for (unsigned int channel = 0; channel < static_cast<unsigned int>(busInput.bus->nChannels); ++channel)
			{
				if (busInput.busToBusMap.mapping[channel].empty()) continue;

				// Copy the bus buffer to this buffer
				// for expanding the amount of channels that the bus has to the amount of channels of the bus
				// so that we can apply panning to Mono buses.
				std::vector<float>& channelInputBuffer = busInfo[bus.get()].busInputBuffers[input][channel].buffer;
				for (unsigned int inChannel = 0; inChannel < static_cast<unsigned int>(
					busInput.busToBusMap.mapping[channel].size()); ++inChannel)
					Detail::SimdHelper::CopyBuffer(
						busInfo[busInput.bus.get()].mainBusBuffer.buffer.data(),
						channelInputBuffer.data(),
						channel * nFrames, inChannel * nFrames, nFrames);

				// TODO: Support Surround Panning
				// Apply panning (for panning Mono tracks to Stereo buses)
				if (busInput.busToBusMap.mapping[channel].size() == static_cast<std::size_t>(TrackState::ChannelNumber::Stereo)
					&& busInput.bus->nChannels == TrackState::ChannelNumber::Mono)
					ApplyStereoPanning(busInput.bus->pan, channelInputBuffer,
						busInput.busToBusMap.mapping[channel].size(),
						nFrames);

				// Go through each output for this track channel (one track channel -> multiple bus channel mapping)
				unsigned int srcChannel = 0;
				for (unsigned int dstChannel : busInput.busToBusMap.mapping[channel])
				{
					// Finally, pass the bus output to the destination bus buffer as the input
					std::vector<float>& busOutputBuffer = busInfo[bus.get()].mainBusBuffer.buffer;

					// busOutputBuffer[busChannel] += channelOutputBuffer[trackChannel]
					Detail::SimdHelper::AccumulateBuffer(
						channelInputBuffer.data(),
						busOutputBuffer.data(),
						srcChannel * nFrames, dstChannel * nFrames,
						nFrames);

					++srcChannel;
				}
			}
		}

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

		// Copy tracks and buses to new vectors just incase any changes are made on a separate thread during processing.
		const std::vector<std::shared_ptr<TrackState::Track>> tracks = audioEngine.trackState.ThreadedCopyAllTracks();
		const std::vector<std::shared_ptr<TrackState::Bus>> buses = audioEngine.trackState.ThreadedCopyAllBuses();

		if (!doTestTone)
		{
			std::lock_guard<std::mutex> lock(audioProcessingMutex);

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
				mixableInfo[track.get()].processAsync = trackThreads.Queue(
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

			// Process Buses (one thread per bus)
			busThreads.Resize(buses.size());
			for (const std::shared_ptr<TrackState::Bus>& bus : buses)
			{
				mixableInfo[bus.get()].processAsync = busThreads.Queue(
					[&]()
					{
						ProcessBus(bus, nFrames, nOutChannels, sampleRate);
					});
			}

			// Send buses to output
			for (const std::shared_ptr<TrackState::Bus>& bus : buses)
			{
				mixableInfo[bus.get()].processAsync.wait();

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

		// Cancel all pending tasks and make sure all the currently executing tasks finish.
		// (would be better if there was a way to cancel currently executing tasks)
		trackThreads.CancelPending();
		for (auto& pair : mixableInfo)
			pair.second.processAsync.wait();

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
				// of the first places I'd look. As this is called 
				// every single audio frame, and we're basically using a 
				// vector as a circular buffer. And while it'd be nice to 
				// use a more optimal data structure like an actual circular 
				// buffer, in this case the size can actually change, 
				// and we need the memory to be contiguous.
				// One idea is instead of using the erase function,
				// we could write a SIMD vector shift function to shift 
				// everything over by nFrames, making room for the new data.
				// However there could still be a more optimal way.
				// 
				// Another way would be to actually allocate more than you need,
				// but only use a small portion of the buffer, then just move 
				// everything back to the beginning of the buffer when it gets full.
				// That way you'd only have to move things around every so often, 
				// and not every single addition essentially.
				// (remember we need to get the peaks and averages, 
				// but VSTs and other audio effects will need this 
				// to be in the correct order too)

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
