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
			std::vector<std::vector<unsigned int>> inputChannelToOutputChannels;

			BusOutput(unsigned int nChannels)
			{
				this->bus = -1;
			}

			BusOutput(BusIdentifier bus, const std::vector<std::vector<unsigned int>>& outputChannels)
			{
				this->bus = bus;
				this->inputChannelToOutputChannels = outputChannels;
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

			Mixable(ChannelNumber nChannels, float gain, float pan, const std::vector<BusOutput>& outputs)
			{
				this->nChannels = nChannels;
				this->gain = gain;
				this->pan = pan;
				this->outputs = outputs;
			}
		};

		// A Track is simply a stream of audio with gain, panning, and VST effects / input (coming some day), plus the ability to output to any number of bus channels.
		struct Track : Mixable
		{
			// TODO: Other track specific features.

			Track()
			{
			}

			Track(ChannelNumber nChannels, float gain, float pan, const std::vector<BusOutput>& outputs)
				: Mixable(nChannels, gain, pan, outputs)
			{
			}
		};

		// A Bus is the same as a track except it can also recieve other tracks / buses as inputs + can output to the current output device / buffer (if exporting) 
		// if specified (as well as output to other buses)
		struct Bus : Mixable
		{
			std::vector<std::vector<unsigned int>> inputChannelToDeviceOutputChannels;

			Bus()
			{
			}

			Bus(ChannelNumber nChannels, float gain, float pan, const std::vector<BusOutput>& outputs, const std::vector<std::vector<unsigned int>>& deviceOutputs)
				: Mixable(nChannels, gain, pan, outputs)
			{
				this->inputChannelToDeviceOutputChannels = deviceOutputs;
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

		Track defaultTrack;
		Bus defaultBus;

		void UpdateTracks();
		void UpdateBuses();
	public:
		TrackState();

		TrackIdentifier AddTrack(Track track);
		BusIdentifier AddBus(Bus bus);

		void RemoveTrack(TrackIdentifier track);
		void RemoveBus(BusIdentifier bus);

		Track& GetTrack(TrackIdentifier track);
		Bus& GetBus(BusIdentifier bus);

		const std::vector<TrackIdentifier>& GetAllTracks();
		const std::vector<BusIdentifier>& GetAllBuses();

		void RegisterUpdateTracksHandler(std::function<void()> handler);
		void RegisterUpdateBusesHandler(std::function<void()> handler);
	};
}
