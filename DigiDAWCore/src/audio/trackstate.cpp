#include "digidaw/core/audio/trackstate.h"

namespace DigiDAW::Core::Audio
{
	// Technically through repeated adding and removing tracks, this could overflow.
	// But you'd have to do that so many times (considering the fact the identifier is 32-bit) that it is pretty much irrelevant.
	TrackState::Track& TrackState::AddTrack(Track track)
	{
		currentTracks.push_back(track);
		for (auto& func : addTrackCallbacks) func(currentTracks.back());
		return currentTracks.back();
	}

	TrackState::Bus& TrackState::AddBus(Bus bus)
	{
		currentBuses.push_back(bus);
		for (auto& func : addBusCallbacks) func(currentBuses.back());
		return currentBuses.back();
	}

	void TrackState::RemoveTrack(Track& track)
	{
		currentTracks.erase(std::find(currentTracks.begin(), currentTracks.end(), track));
		for (auto& func : removeTrackCallbacks) func(track);
	}

	void TrackState::RemoveBus(Bus& bus)
	{
		currentBuses.erase(std::find(currentBuses.begin(), currentBuses.end(), bus));
		for (auto& func : removeBusCallbacks) func(bus);
	}

	const std::vector<TrackState::Track>& TrackState::GetAllTracks()
	{
		return currentTracks;
	}

	const std::vector<TrackState::Bus>& TrackState::GetAllBuses()
	{
		return currentBuses;
	}
}
