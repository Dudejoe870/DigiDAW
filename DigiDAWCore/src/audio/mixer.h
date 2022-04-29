#pragma once

#include "audio/common.h"

#include "audio/trackstate.h"

namespace DigiDAW::Audio
{
	class Engine;

	class Mixer
	{
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

		std::unordered_map<TrackState::TrackIdentifier, MixBuffer> trackBuffers;
		std::unordered_map<TrackState::BusIdentifier, MixBuffer> busBuffers;

		void ApplyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void ApplyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void ProcessTrack(std::vector<float>& trackInputBuffer, TrackState::TrackIdentifier track, unsigned int nFrames, unsigned int sampleRate);
		void ProcessBus(TrackState::BusIdentifier bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate);
	public:
		Mixer(Engine& audioEngine);

		void UpdateTrackBuffers();
		void UpdateBusBuffers();

		void Mix(
			float* outputBuffer,
			float* inputBuffer, 
			double time, 
			unsigned int nFrames, 
			unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate);

		void UpdateCurrentTime(double time);

		void StartTestTone();
		void EndTestTone();
	};
}
