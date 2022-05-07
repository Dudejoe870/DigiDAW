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
		struct ChannelInfo
		{
			float averageAmplitude;

			ChannelInfo()
			{
				this->averageAmplitude = 0.0f;
			}
		};

		struct MixableInfo
		{
			std::vector<ChannelInfo> channels;

			MixableInfo()
			{
			}
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
				buffer.resize(nChannels * nFrames);
			}
		};
		
		struct TrackBuffers
		{
			MixBuffer mainTrackBuffer;
			std::vector<std::vector<MixBuffer>> busOutputBuffers;
			
			TrackBuffers()
			{
			}

			TrackBuffers(MixBuffer mainTrackBuffer, std::vector<std::vector<MixBuffer>>& busOutputBuffers)
			{
				this->mainTrackBuffer = mainTrackBuffer;
				this->busOutputBuffers = busOutputBuffers;
			}
		};

		std::unordered_map<const TrackState::Track*, TrackBuffers> trackBuffers;
		std::unordered_map<const TrackState::Bus*, MixBuffer> busBuffers;

		std::unordered_map<const TrackState::Mixable*, MixableInfo> mixableInfo;

		std::vector<ChannelInfo> outputChannels;

		double streamDeltaTime = 0.0;
		double streamLastTime = 0.0;
		double updateCounter = 0.0;

		TrackBuffers GetTrackBuffers(const TrackState::Track& track, unsigned int nFrames, unsigned int nChannels);

		void ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);

		void ProcessTrack(std::vector<float>& trackInputBuffer, const TrackState::Track& track, unsigned int nFrames, unsigned int sampleRate);
		void ProcessBus(const TrackState::Bus& bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate);
	public:
		double updateInterval = 0.03;

		Mixer(Engine& audioEngine);

		void UpdateAllTrackBuffers();
		void UpdateAllBusBuffers();

		void Mix(
			float* outputBuffer,
			float* inputBuffer, 
			double time, 
			unsigned int nFrames, 
			unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate);

		void UpdateCurrentTime(double time);

		void StartTestTone();
		void EndTestTone();

		const MixableInfo& GetMixableInfo(const TrackState::Mixable& mixable)
		{
			return mixableInfo[&mixable];
		}

		const std::vector<ChannelInfo>& GetOutputChannels()
		{
			return outputChannels;
		}
	};
}
