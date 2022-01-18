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
			float* buffer;

			MixBuffer()
			{
				buffer = nullptr;
			}

			MixBuffer(unsigned int nFrames, unsigned int nChannels)
			{
				buffer = new float[nChannels * nFrames];
			}

			~MixBuffer()
			{
				if (buffer) delete[] buffer;
			}
		};

		std::unordered_map<TrackState::TrackIdentifier, MixBuffer> trackBuffers;
		std::unordered_map<TrackState::BusIdentifier, MixBuffer> busBuffers;

		void updateTrackBuffers();
		void updateBusBuffers();
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
