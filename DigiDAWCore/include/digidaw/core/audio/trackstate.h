#pragma once

#include "digidaw/core/audio/common.h"

namespace DigiDAW::Core::Audio
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

		struct Bus;

		struct BusOutput
		{
			std::reference_wrapper<Bus> bus;
			std::vector<std::vector<unsigned int>> inputChannelToOutputChannels;

			BusOutput(Bus& bus, const std::vector<std::vector<unsigned int>>& outputChannels)
				: bus(bus)
			{
				this->inputChannelToOutputChannels = outputChannels;
			}
		};

		struct Mixable
		{
			ChannelNumber nChannels;

			float gain;
			float pan;

			std::string name;

			Mixable()
			{
				this->name = "";

				this->nChannels = ChannelNumber::Mono;
				this->gain = 0.0f;
				this->pan = 0.0f;
			}

			Mixable(std::string name, ChannelNumber nChannels, float gain, float pan)
			{
				this->name = name;

				this->nChannels = nChannels;
				this->gain = gain;
				this->pan = pan;
			}

			inline bool operator==(const Mixable& rhs) { return this == &rhs; }
		};

		// A Track is simply a stream of audio with gain, panning, and VST effects / input (coming some day), plus the ability to output to any number of bus channels.
		struct Track : Mixable
		{
			// TODO: Other track specific features.

			std::vector<BusOutput> outputs;

			Track()
			{
			}

			Track(std::string name, ChannelNumber nChannels, float gain, float pan, const std::vector<BusOutput>& outputs)
				: Mixable(name, nChannels, gain, pan)
			{
				this->outputs = outputs;
			}

			inline bool operator==(const Track& rhs) { return this == &rhs; }
		};

		// A Bus is the same as a track except it recieves other tracks as inputs + can output to the current output device / buffer (if exporting) 
		// if specified (as well as output to other buses)
		struct Bus : Mixable
		{
			std::vector<std::vector<unsigned int>> busChannelToDeviceOutputChannels;

			Bus()
			{
			}

			Bus(std::string name, ChannelNumber nChannels, float gain, float pan, const std::vector<std::vector<unsigned int>>& deviceOutputs)
				: Mixable(name, nChannels, gain, pan)
			{
				this->busChannelToDeviceOutputChannels = deviceOutputs;
			}

			inline bool operator==(const Bus& rhs) { return this == &rhs; }
		};
	private:
		std::vector<Track> currentTracks;
		std::vector<Bus> currentBuses;
	public:
		std::vector<std::function<void(Track&)>> addTrackCallbacks;
		std::vector<std::function<void(Track&)>> removeTrackCallbacks;

		std::vector<std::function<void(Bus&)>> addBusCallbacks;
		std::vector<std::function<void(Bus&)>> removeBusCallbacks;

		Track& AddTrack(Track track);
		Bus& AddBus(Bus bus);

		void RemoveTrack(Track& track);
		void RemoveBus(Bus& bus);

		const std::vector<Track>& GetAllTracks();
		const std::vector<Bus>& GetAllBuses();
	};
}
