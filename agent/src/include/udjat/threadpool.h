/**
 * @file src/include/udjat/agent.h
 *
 * @brief Declare the agent classes
 *
 * @author perry.werneck@gmail.com
 *
 */

#ifndef UDJAT_THREADPOOL_H_INCLUDED

	#define UDJAT_THREADPOOL_H_INCLUDED

	#include <udjat/defs.h>
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <queue>
	#include <condition_variable>
	#include <functional>
	#include <pugixml.hpp>

	namespace Udjat {

		class DLL_PUBLIC ThreadPool {
		private:

			/// @brief Lock guard to prevent multiple accesses to the queue.
			std::mutex guard;

			/// @brief Task queue.
			std::queue<std::function<void()>> tasks;

			struct {
				std::atomic<size_t>	  active;		///< @brief Number of active threads.
				std::atomic<size_t>	  waiting;		///< @brief Número of idle threads.
			} threads;

			static void worker(ThreadPool *pool) noexcept;

			bool pop(std::function<void()> &cbk) noexcept;

#ifdef _WIN32

#else
			struct {
				std::mutex m;
				std::condition_variable cv;
			} event;

			inline void wakeup() noexcept {
				event.cv.notify_one();
			}

#endif // _WIN32

		protected:

			struct {
				size_t threads;	///< @brief Limit the number of threads.
				size_t tasks;	///< @brief Limit the number of tasks.
				size_t idle;	///< @brief How many seconds a thread stay idle before finish.
			} limits;

		public:

			static ThreadPool & getInstance();

			ThreadPool();
			ThreadPool(const pugi::xml_node &node);
			~ThreadPool();

			void stop();
			void set(const pugi::xml_node &node);

			inline operator bool() const noexcept {
				return threads.active > 0;
			}

			size_t push(std::function<void()> callback);

		};

	}

#endif // UDJAT_THREADPOOL_H_INCLUDED
