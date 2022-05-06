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

		/*
		 * The idea for this Track architecture is to use buses to output to the output device channels
		 * AND act as a way to organize groups of tracks (for example, you could have a bus specifically for drums, 
		 * or a stereo setup for recording Piano, etc.) Which is a little different to how other DAWs work, 
		 * as aux tracks are usually what you use to group tracks together, and secondary buses usually output to the main bus, 
		 * while in this case, buses can only output to the main output device. 
		 * As such it's easier to understand the relationship between different types of "Mixables", 
		 * and is more straightforward to organize things (In combination with Track folders).
		 */

		// A Track is simply a stream of audio (from the input device or from the output of a VST driven with MIDI) 
		// with gain, panning, and audio effects (including VSTs), plus the ability to output to any number of bus channels.
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

		// A Bus is the same as a track except it recieves other tracks as inputs + can output to the 
		// current output device / buffer (if exporting) if specified.
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

		std::vector<TrackState::Track>& GetAllTracks()
		{
			return currentTracks;
		}

		std::vector<TrackState::Bus>& GetAllBuses()
		{
			return currentBuses;
		}
	};
}
