#include "audio/trackstate.h"

#pragma once

#include "audio/common.h"

namespace DigiDAW::Audio
{
	TrackState::TrackState()
	{
		this->currentTrackIndex = 0;
		this->currentBusIndex = 0;
	}

	void TrackState::UpdateTracks()
	{
		std::vector<TrackIdentifier> identifiers;
		for (auto& kv : tracks) identifiers.push_back(kv.first);

		currentTracks = identifiers;

		for (auto& func : updateTracksCallbacks) func();
	}

	void TrackState::UpdateBuses()
	{
		std::vector<BusIdentifier> identifiers;
		for (auto& kv : buses) identifiers.push_back(kv.first);

		currentBuses = identifiers;

		for (auto& func : updateBusesCallbacks) func();
	}

	// Technically through repeated adding and removing tracks, this could overflow.
	// But you'd have to do that so many times (considering the fact the identifier is 64-bit) that it is pretty much irrelevant.
	TrackState::TrackIdentifier TrackState::AddTrack(Track track)
	{
		tracks[currentTrackIndex] = track;
		UpdateTracks();
		return currentTrackIndex++;
	}

	TrackState::BusIdentifier TrackState::AddBus(Bus bus)
	{
		buses[currentBusIndex] = bus;
		UpdateBuses();
		return currentBusIndex++;
	}

	void TrackState::RemoveTrack(TrackIdentifier track)
	{
		tracks.erase(track);
		UpdateTracks();
	}

	void TrackState::RemoveBus(BusIdentifier bus)
	{
		buses.erase(bus);
		UpdateBuses();
	}

	TrackState::Track& TrackState::GetTrack(TrackIdentifier track)
	{
		if (track == -1 || !tracks.contains(track))
			return defaultTrack;
		return tracks[track];
	}

	TrackState::Bus& TrackState::GetBus(BusIdentifier bus)
	{
		if (bus == -1 || !buses.contains(bus))
			return defaultBus;
		return buses[bus];
	}

	const std::vector<TrackState::TrackIdentifier>& TrackState::GetAllTracks()
	{
		return currentTracks;
	}

	const std::vector<TrackState::BusIdentifier>& TrackState::GetAllBuses()
	{
		return currentBuses;
	}

	void TrackState::RegisterUpdateTracksHandler(std::function<void()> handler)
	{
		updateTracksCallbacks.push_back(handler);
	}

	void TrackState::RegisterUpdateBusesHandler(std::function<void()> handler)
	{
		updateBusesCallbacks.push_back(handler);
	}
}
