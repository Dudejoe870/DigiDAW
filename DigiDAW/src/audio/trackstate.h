#pragma once

#include "audio/common.h"

namespace DigiDAW::Audio
{
	class TrackState
	{
	public:
		enum class ChannelNumber
		{
			Mono = 1,
			Stereo = 2,
			Surround_5_1 = 6,
			Surround_7_1 = 8,

			MAX = Surround_7_1
		};

		typedef unsigned long TrackIdentifier;
		typedef unsigned long BusIdentifier;

		struct BusOutput
		{
			BusIdentifier bus;
			std::unordered_map<unsigned int, unsigned int> inputToOutputChannelMapping;

			BusOutput()
			{
				this->bus = -1;
			}
		};

		struct Mixable
		{
			ChannelNumber nChannels;

			float gain;
			float pan;

			std::vector<BusOutput> outputs;

			Mixable()
			{
				this->nChannels = ChannelNumber::Mono;
				this->gain = 0.0f;
				this->pan = 0.0f;
			}
		};

		// A Track is simply a stream of audio with gain, panning, and VST effects / input (coming some day), plus the ability to output to any number of bus channels.
		struct Track : Mixable
		{
			// TODO: Other track specific features.
		};

		// A Bus is the same as a track except it can also recieve other tracks / buses as inputs + can output to the current output device / buffer (if exporting) 
		// if specified (as well as output to other buses)
		struct Bus : Mixable
		{
			unsigned int outputDeviceChannels[(unsigned int)ChannelNumber::MAX];

			Bus()
			{
				for (int i = 0; i < sizeof(outputDeviceChannels) / sizeof(int); ++i)
					outputDeviceChannels[i] = -1;
			}
		};
	private:
		TrackIdentifier currentTrackIndex;
		BusIdentifier currentBusIndex;

		std::unordered_map<TrackIdentifier, Track> tracks;
		std::unordered_map<BusIdentifier, Bus> buses;

		std::vector<TrackIdentifier> currentTracks;
		std::vector<BusIdentifier> currentBuses;

		std::vector<std::function<void()>> updateTracksCallbacks;
		std::vector<std::function<void()>> updateBusesCallbacks;

		void updateTracks();
		void updateBuses();
	public:
		TrackState();

		TrackIdentifier addTrack(Track track);
		BusIdentifier addBus(Bus bus);

		void removeTrack(TrackIdentifier track);
		void removeBus(BusIdentifier bus);

		Track getTrack(TrackIdentifier track);
		Bus getBus(BusIdentifier bus);

		const std::vector<TrackIdentifier>& getAllTracks();
		const std::vector<BusIdentifier>& getAllBuses();

		void registerUpdateTracksHandler(std::function<void()> handler);
		void registerUpdateBusesHandler(std::function<void()> handler);
	};
}
