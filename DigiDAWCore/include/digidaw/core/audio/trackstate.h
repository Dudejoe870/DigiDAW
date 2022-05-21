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
			std::shared_ptr<Bus> bus;
			std::vector<std::vector<unsigned int>> inputChannelToOutputChannels;

			BusOutput(std::shared_ptr<Bus>& bus, const std::vector<std::vector<unsigned int>>& outputChannels)
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

			char name[256];

			Mixable()
			{
				std::memset(this->name, 0, sizeof(this->name));

				this->nChannels = ChannelNumber::Mono;
				this->gain = 0.0f;
				this->pan = 0.0f;
			}

			Mixable(const std::string& name, ChannelNumber nChannels, float gain, float pan)
			{
				std::memset(this->name, 0, sizeof(this->name));
				if (name.size() < 255 /* 255 as the end of the name needs to be a zero */) 
					std::memcpy(this->name, name.c_str(), name.size());

				this->nChannels = nChannels;
				this->gain = gain;
				this->pan = pan;
			}
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

			// TODO: Move these into the Bus struct as INPUTS. That makes it even more parallelizable, 
			// (as we can just spin up a thread per Bus, 
			// wait on the specified tracks to finish then accumulate them all and then apply gain / panning)
			// AND it allows us to also do bus to bus inputs potentially. 
			// That allows for the scenario that we don't have to wait on all the tracks to finish processing 
			// before processing certain buses in parallel. Perhaps having a profound performance impact (in some situations)
			std::vector<BusOutput> outputs;

			Track()
			{
			}

			Track(const std::string& name, ChannelNumber nChannels, float gain, float pan, const std::vector<BusOutput>& outputs)
				: Mixable(name, nChannels, gain, pan)
			{
				this->outputs = outputs;
			}
		};

		// A Bus is the same as a track except it recieves other tracks as inputs + can output to the 
		// current output device / buffer (if exporting) when specified.
		struct Bus : Mixable
		{
			std::vector<std::vector<unsigned int>> busChannelToDeviceOutputChannels;

			Bus()
			{
			}

			Bus(const std::string& name, ChannelNumber nChannels, float gain, float pan, const std::vector<std::vector<unsigned int>>& deviceOutputs)
				: Mixable(name, nChannels, gain, pan)
			{
				this->busChannelToDeviceOutputChannels = deviceOutputs;
			}
		};
	private:
		std::vector<std::shared_ptr<Track>> currentTracks;
		std::vector<std::shared_ptr<Bus>> currentBuses;
	public:
		std::vector<std::function<void(std::shared_ptr<Track>)>> addTrackCallbacks;
		std::vector<std::function<void(std::shared_ptr<Track>)>> removeTrackCallbacks;

		std::vector<std::function<void(std::shared_ptr<Bus>)>> addBusCallbacks;
		std::vector<std::function<void(std::shared_ptr<Bus>)>> removeBusCallbacks;

		std::shared_ptr<Track> AddTrack(Track track);
		std::shared_ptr<Bus> AddBus(Bus bus);

		void RemoveTrack(std::shared_ptr<Track> track);
		void RemoveBus(std::shared_ptr<Bus> bus);

		std::vector<std::shared_ptr<TrackState::Track>>& GetAllTracks()
		{
			return currentTracks;
		}

		std::vector<std::shared_ptr<TrackState::Bus>>& GetAllBuses()
		{
			return currentBuses;
		}
	};
}
