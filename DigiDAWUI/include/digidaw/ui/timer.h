#pragma once

#include <vector>
#include <functional>

namespace DigiDAW::UI
{
	class Timer
	{
	private:
		using T_TimerElapsed = std::function<void()>;

		struct PendingTimer
		{
			float seconds;
			T_TimerElapsed timerElapsed;

			float elapsed;

			PendingTimer()
			{
				this->seconds = 0.0f;
				this->timerElapsed = nullptr;
				this->elapsed = 0.0f;
			}

			PendingTimer(float seconds, T_TimerElapsed timerElapsed)
			{
				this->seconds = seconds;
				this->timerElapsed = timerElapsed;
				this->elapsed = 0.0f;
			}
		};

		static std::unordered_map<unsigned long long, PendingTimer> pendingTimers;
	public:
		static void AddTimer(unsigned long long id, float seconds, T_TimerElapsed timerElapsed);

		static void UpdatePendingTimers();
	};
}
