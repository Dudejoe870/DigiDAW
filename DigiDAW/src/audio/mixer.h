#pragma once

#include "audio/common.h"

namespace DigiDAW::Audio
{
	class Mixer
	{
	private:
		bool doTestTone;

		double testToneStartTime;
		double currentTime;
	public:
		class MixingState
		{
		public:
			enum class ChannelType
			{
				Mono,
				Stereo
			};

			typedef unsigned long TrackIdentifier;
			typedef unsigned long BusIdentifier;

			struct Mixable
			{
				ChannelType type;

				float gain;
				float pan;
			};

			struct Track : Mixable
			{
				// TODO: Track sends

				BusIdentifier output;
			};

			struct Bus : Mixable
			{
				BusIdentifier busOutput;
				unsigned int channelOutput[2]; // If busOutput is -1, this outputs to the current output device using the specified channels (both if Stereo, only the first one if Mono)
			};
		private:
			std::unordered_map<TrackIdentifier, Track> tracks;
			std::unordered_map<BusIdentifier, Bus> buses;
		public:

		};

		MixingState mixingState;

		Mixer();

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
