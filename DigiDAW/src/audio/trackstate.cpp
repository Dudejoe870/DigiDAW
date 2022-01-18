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

	void TrackState::updateTracks()
	{
		std::vector<TrackIdentifier> identifiers(tracks.size());
		for (auto& kv : tracks) identifiers.push_back(kv.first);

		currentTracks = identifiers;

		for (auto& func : updateTracksCallbacks) func();
	}

	void TrackState::updateBuses()
	{
		std::vector<BusIdentifier> identifiers(buses.size());
		for (auto& kv : buses) identifiers.push_back(kv.first);

		currentBuses = identifiers;

		for (auto& func : updateBusesCallbacks) func();
	}

	// Technically through repeated adding and removing tracks, this could overflow.
	// But you'd have to do that so many times (considering the fact the identifier is 64-bit) that it is pretty much irrelevant.
	TrackState::TrackIdentifier TrackState::addTrack(Track track)
	{
		tracks[currentTrackIndex] = track;
		updateTracks();
		return currentTrackIndex++;
	}

	TrackState::BusIdentifier TrackState::addBus(Bus bus)
	{
		buses[currentBusIndex] = bus;
		updateBuses();
		return currentBusIndex++;
	}

	void TrackState::removeTrack(TrackIdentifier track)
	{
		tracks.erase(track);
		updateTracks();
	}

	void TrackState::removeBus(BusIdentifier bus)
	{
		buses.erase(bus);
		updateBuses();
	}

	TrackState::Track TrackState::getTrack(TrackIdentifier track)
	{
		if (track == -1 || !tracks.contains(track))
			return Track();
		return tracks[track];
	}

	TrackState::Bus TrackState::getBus(BusIdentifier bus)
	{
		if (bus == -1 || !buses.contains(bus))
			return Bus();
		return buses[bus];
	}

	const std::vector<TrackState::TrackIdentifier>& TrackState::getAllTracks()
	{
		return currentTracks;
	}

	const std::vector<TrackState::BusIdentifier>& TrackState::getAllBuses()
	{
		return currentBuses;
	}

	void TrackState::registerUpdateTracksHandler(std::function<void()> handler)
	{
		updateTracksCallbacks.push_back(handler);
	}

	void TrackState::registerUpdateBusesHandler(std::function<void()> handler)
	{
		updateBusesCallbacks.push_back(handler);
	}
}
