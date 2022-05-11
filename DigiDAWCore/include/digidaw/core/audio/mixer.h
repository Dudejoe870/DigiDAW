#pragma once

#include "digidaw/core/audio/common.h"

#include "digidaw/core/audio/trackstate.h"

namespace DigiDAW::Core::Audio
{
	class Engine;

	/*
	 * The audio mixing chain for each track is like this:
	 * 
	 * track input buffer (either an input channel from the input device or some other source like a VST)
	 * |
	 * track buffer (this is where gain is applied and panning if it's a stereo track)
	 * |
	 * channel output buffer 
	 * (this is apart of the track -> bus channel mapping system, 
	 * which allows any channel on the track to be mapped to any number of bus channels. 
	 * Panning gets applied here instead if this is a mono track with a stereo output)
	 * |
	 * bus buffer 
	 * (this is where bus processing begins, any track can output to any number of buses.
	 * bus gain and bus panning is applied here)
	 */
	class Mixer
	{
	public:
		// TODO: LUFS metering (could be based off this https://github.com/klangfreund/LUFSMeter)
		struct ChannelInfo
		{
		public:
			float rms;
			float peak;
		public:
			ChannelInfo()
			{
				this->rms = -(float)INFINITY;
				this->peak = -(float)INFINITY;
			}

			friend class Mixer;
		};

		struct MixableInfo
		{
		public:
			std::vector<ChannelInfo> channels;
		private:
			std::vector<std::vector<float>> lookbackBuffers; // The lookback buffers that are used to calculate the amplitudes used for metering.
		public:
			MixableInfo()
			{
			}

			friend class Mixer;
		};
	private:
		bool doTestTone;

		double testToneStartTime;
		double currentTime;

		Engine& audioEngine;

		struct MixBuffer
		{
			std::vector<float> buffer;

			MixBuffer()
			{
			}

			MixBuffer(unsigned int nFrames, unsigned int nChannels)
			{
				this->buffer = std::vector<float>(nChannels * nFrames);
			}
		};
		
		struct TrackInfo
		{
			MixBuffer mainTrackBuffer;
			std::vector<std::vector<MixBuffer>> busOutputBuffers;
			std::future<void> processAsync;

			TrackInfo()
			{
			}

			TrackInfo(Engine& audioEngine, const TrackState::Track& track, unsigned int nFrames);
		};

		struct BusInfo
		{
			MixBuffer mainBusBuffer;

			BusInfo()
			{

			}

			BusInfo(const TrackState::Bus& bus, unsigned int nFrames)
			{
				this->mainBusBuffer = MixBuffer(nFrames, (unsigned int)bus.nChannels);
			}
		};

		std::unordered_map<const TrackState::Track*, TrackInfo> trackInfo;
		std::unordered_map<const TrackState::Bus*, BusInfo> busInfo;
		std::unordered_map<const TrackState::Mixable*, MixableInfo> mixableInfo;

		MixableInfo outputInfo;
		unsigned int nOutChannels;

		bool running = true;
		bool shouldAddToLookback = true;
		std::jthread mixerThread;

		void LerpMeter(float& value, const float& target, float deltaTime, float riseTime, float fallTime, float minimumValue)
		{
			float time = (target > value) ? riseTime : fallTime;
			float delta = std::abs(std::max(value, target) - std::min(value, target));
			float timeToChange = (delta / -minimumValue /* Make the minimum value positive */) * time;
			float t = std::clamp(deltaTime / timeToChange, 0.0f, 1.0f);

			value = std::max(std::lerp(value, target, t), minimumValue);
		}

		void AddToLookback(float* src, std::vector<std::vector<float>>& dst, unsigned nFrames, unsigned int nChannels, unsigned int sampleRate);

		void ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);

		void ProcessTrack(std::vector<float>& trackInputBuffer, const TrackState::Track& track, unsigned int nFrames, unsigned int sampleRate);
		void ProcessBus(const TrackState::Bus& bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate);
	public:
		unsigned int meterUpdateIntervalMS = 16;
		unsigned int lookbackBufferIntervalMS = 100; // The amount of time the meter uses to average over

		unsigned int meterRMSRiseTimeMS = 36; // The time it takes to go from minimum to maximum dB
		unsigned int meterRMSFallTimeMS = 500; // The time it takes to go from maximum to minimum dB

		unsigned int meterPeakRiseTimeMS = 40;
		unsigned int meterPeakFallTimeMS = 1000;

		float minimumDecibelLevel = -60.0f;

		Mixer(Engine& audioEngine);
		~Mixer();

		void UpdateAllTrackBuffers();
		void UpdateAllBusBuffers();

		void Mix(
			float* outputBuffer,
			float* inputBuffer, 
			double time, 
			unsigned int nFrames, 
			unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate);

		void UpdateCurrentTime(double time)
		{
			this->currentTime = time;
		}

		void StartTestTone();
		void EndTestTone();

		const MixableInfo& GetMixableInfo(const TrackState::Mixable& mixable)
		{
			return mixableInfo[&mixable];
		}

		const MixableInfo& GetOutputInfo()
		{
			return outputInfo;
		}
	};
}
