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
		struct Track;

		struct ChannelMapping
		{
			std::vector<std::vector<unsigned int>> mapping;

			ChannelMapping(const std::vector<std::vector<unsigned int>>& mapping)
			{
				this->mapping = mapping;
			}
		};

		struct TrackInput
		{
			std::shared_ptr<Track> track;
			ChannelMapping trackToBusMap;

			TrackInput(std::shared_ptr<Track>& track, const ChannelMapping& trackToBusMap)
				: trackToBusMap(trackToBusMap)
			{
				this->track = track;
			}
		};

		struct BusInput
		{
			std::shared_ptr<Bus> bus;
			ChannelMapping busToBusMap;

			BusInput(std::shared_ptr<Bus>& bus, const ChannelMapping& busToBusMap)
				: busToBusMap(busToBusMap)
			{
				this->bus = bus;
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
		 * while in this case, buses can only output to the main output device + other buses. 
		 * As such it's easier to understand the relationship between different types of "Mixables", 
		 * and is more straightforward to organize things (In combination with Track folders).
		 */

		// A Track is simply a stream of audio (from the input device or from the output of a VST driven with MIDI) 
		// with gain, panning, and audio effects (including VSTs), plus the ability to output to any number of bus channels.
		struct Track : Mixable
		{
			// TODO: Other track specific features.

			Track()
			{
			}

			Track(const std::string& name, ChannelNumber nChannels, float gain, float pan)
				: Mixable(name, nChannels, gain, pan)
			{
			}
		};

		// A Bus is the same as a track except it recieves other tracks as inputs + can output to the 
		// current output device / buffer (if exporting) when specified.
		struct Bus : Mixable
		{
			std::vector<TrackInput> trackInputs;
			std::vector<BusInput> busInputs;

			std::vector<std::vector<unsigned int>> busChannelToDeviceOutputChannels;

			bool CheckCircularBusDependency(std::shared_ptr<Bus> otherBus)
			{
				for (const BusInput& input : otherBus->busInputs)
				{
					if (input.bus.get() == this)
						return true;
					// Check that the other bus doesn't also have a track that we have as an input.
					for (const TrackInput& sourceTrackInput : input.bus->trackInputs)
						for (const TrackInput& destTrackInput : trackInputs)
							if (sourceTrackInput.track == destTrackInput.track) 
								return true;
				}
				return false;
			}

			void ValidateAndRemoveInvalidInputs()
			{
				std::erase_if(busInputs, 
					[this](const BusInput& input) { return CheckCircularBusDependency(input.bus); });
			}

			Bus()
			{
			}

			Bus(const std::string& name, ChannelNumber nChannels, float gain, float pan, 
				const std::vector<std::vector<unsigned int>>& deviceOutputs,
				const std::vector<TrackInput>& trackInputs,
				const std::vector<BusInput>& busInputs)
				: Mixable(name, nChannels, gain, pan)
			{
				this->busChannelToDeviceOutputChannels = deviceOutputs;
				this->trackInputs = trackInputs;
				this->busInputs = busInputs;

				ValidateAndRemoveInvalidInputs();
			}
		};
	private:
		std::vector<std::shared_ptr<Track>> currentTracks;
		std::vector<std::shared_ptr<Bus>> currentBuses;

		std::mutex tracksMutex;
		std::mutex busesMutex;
	public:
		std::vector<std::function<void(std::shared_ptr<Track>)>> addTrackCallbacks;
		std::vector<std::function<void(std::shared_ptr<Track>)>> removeTrackCallbacks;

		std::vector<std::function<void(std::shared_ptr<Bus>)>> addBusCallbacks;
		std::vector<std::function<void(std::shared_ptr<Bus>)>> removeBusCallbacks;

		std::shared_ptr<Track> AddTrack(Track track);
		std::shared_ptr<Bus> AddBus(Bus bus);

		void RemoveTrack(std::shared_ptr<Track>& track);
		void RemoveBus(std::shared_ptr<Bus>& bus);

		std::vector<std::shared_ptr<TrackState::Track>>& GetAllTracks()
		{
			return currentTracks;
		}

		std::vector<std::shared_ptr<TrackState::Bus>>& GetAllBuses()
		{
			return currentBuses;
		}

		const std::vector<std::shared_ptr<TrackState::Track>> ThreadedCopyAllTracks()
		{
			std::lock_guard<std::mutex> lock(tracksMutex);
			return std::vector<std::shared_ptr<TrackState::Track>>(currentTracks);
		}

		const std::vector<std::shared_ptr<TrackState::Bus>> ThreadedCopyAllBuses()
		{
			std::lock_guard<std::mutex> lock(busesMutex);
			return std::vector<std::shared_ptr<TrackState::Bus>>(currentBuses);
		}
	};
}
