#pragma once

#include "digidaw/core/common.h"

namespace DigiDAW::Core::Threading
{
	class ThreadPool
	{
	private:
		struct ThreadInfo
		{
			std::future<void> future;
			bool isRunning;

			ThreadInfo()
			{
				isRunning = false;
			}

			ThreadInfo(std::future<void> future)
			{
				this->future = std::move(future);
				isRunning = true;
			}
		};

		std::mutex queueMutex;
		std::condition_variable threadSemaphore;

		std::deque<std::packaged_task<void()>> work;

		std::vector<ThreadInfo> threads;

		void ThreadTask(std::size_t id)
		{
			while (threads[id].isRunning)
			{
				std::packaged_task<void()> task;
				{
					std::unique_lock<std::mutex> lock(queueMutex);
					// Wait until a task is added if the work queue is empty.
					if (work.empty())
						threadSemaphore.wait(
							lock, 
							[&] { return !work.empty() || !threads[id].isRunning; });
					if (!threads[id].isRunning) return;

					// If there is a task, get it out of the queue.
					task = std::move(work.front());
					work.pop_front();
				}
				// If the task is invalid, it means an abort message has been sent.
				if (!task.valid()) return;
				task(); // Otherwise run the task.
			}
		}
	public:
		template<class F, class R = std::invoke_result_t<std::decay_t<F>>>
		std::future<R> Queue(F&& f)
		{
			std::packaged_task<R()> task(std::forward<F>(f));
			
			auto future = task.get_future();
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				work.emplace_back(std::move(task));
			}
			threadSemaphore.notify_one(); // Wake a thread to work on the task

			return future;
		}

		void Resize(std::size_t N)
		{
			if (N == threads.size()) return;

			if (N > threads.size())
			{
				threads.reserve(N);
				for (std::size_t i = threads.size(); i < N; ++i)
					threads.push_back(ThreadInfo(
						std::async(
							std::launch::async, 
							[this, i] { ThreadTask(i); })));
			}
			else if (N < threads.size())
			{
				for (std::size_t i = threads.size(); i >= N; --i)
				{
					threads[i].isRunning = false;
					threads[i].future.wait();
				}
				threads.resize(N);
			}
		}

		ThreadPool(std::size_t N = 1)
		{
			Resize(N);
		}

		void CancelPending()
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			work.clear();
		}

		void Clear()
		{
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				for (ThreadInfo& thread : threads)
					work.push_back({}); // Abort all threads.
			}
			threadSemaphore.notify_all();
			threads.clear();
		}

		~ThreadPool()
		{
			CancelPending();
			Clear();
		}
	};
}
