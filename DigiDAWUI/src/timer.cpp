#include "timer.h"

#include "imgui.h"

#include <algorithm>

namespace DigiDAW::UI
{
	std::unordered_map<unsigned long long, Timer::PendingTimer> Timer::pendingTimers;

	void Timer::AddTimer(unsigned long long id, float seconds, T_TimerElapsed timerElapsed)
	{
		pendingTimers[id] = PendingTimer(seconds, timerElapsed);
	}

	void Timer::UpdatePendingTimers()
	{
		ImGuiIO& io = ImGui::GetIO();
		std::vector<unsigned long long> elapsedTimers;
		for (std::pair<unsigned long long, PendingTimer> pair : pendingTimers)
		{
			pair.second.elapsed += io.DeltaTime;
			if (pair.second.elapsed >= pair.second.seconds)
			{
				pair.second.timerElapsed();
				elapsedTimers.push_back(pair.first);
			}
		}

		for (unsigned long long timer : elapsedTimers)
			pendingTimers.erase(timer);
	}
}
