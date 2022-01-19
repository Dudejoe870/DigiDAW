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

		void updateTrackBuffers();
		void updateBusBuffers();

		void applyGain(float gain, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void applyStereoPanning(float pan, std::vector<float>& buffer, unsigned int nChannels, unsigned int nFrames);
		void processTrack(std::vector<float>& trackInputBuffer, TrackState::TrackIdentifier track, unsigned int nFrames, unsigned int sampleRate);
		void processBus(TrackState::BusIdentifier bus, unsigned int nFrames, unsigned int nOutChannels, unsigned int sampleRate);
	public:
		Mixer(Engine& audioEngine);

		void mix(
			float* outputBuffer,
			float* inputBuffer, 
			double time, 
			unsigned int nFrames, 
			unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate);

		void updateCurrentTime(double time);

		void startTestTone();
		void endTestTone();
	};
}
