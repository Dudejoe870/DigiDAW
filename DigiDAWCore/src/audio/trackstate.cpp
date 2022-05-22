#include "digidaw/core/audio/trackstate.h"

namespace DigiDAW::Core::Audio
{
	std::shared_ptr<TrackState::Track> TrackState::AddTrack(Track track)
	{
		std::lock_guard<std::mutex> lock(tracksMutex);

		currentTracks.push_back(std::make_shared<Track>(track));
		for (auto& func : addTrackCallbacks) func(currentTracks.back());
		return currentTracks.back();
	}

	std::shared_ptr<TrackState::Bus> TrackState::AddBus(Bus bus)
	{
		std::lock_guard<std::mutex> lock(busesMutex);

		currentBuses.push_back(std::make_shared<Bus>(bus));
		for (auto& func : addBusCallbacks) func(currentBuses.back());
		return currentBuses.back();
	}

	void TrackState::RemoveTrack(std::shared_ptr<Track>& track)
	{
		std::lock_guard<std::mutex> lock(tracksMutex);

		currentTracks.erase(std::find(currentTracks.begin(), currentTracks.end(), track));
		for (auto& func : removeTrackCallbacks) func(track);
	}

	void TrackState::RemoveBus(std::shared_ptr<Bus>& bus)
	{
		std::lock_guard<std::mutex> lock(busesMutex);

		currentBuses.erase(std::find(currentBuses.begin(), currentBuses.end(), bus));
		for (auto& func : removeBusCallbacks) func(bus);
	}
}
