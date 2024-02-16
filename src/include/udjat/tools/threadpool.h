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
	#include <udjat/tools/xml.h>
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <queue>
	#include <condition_variable>
	#include <functional>

	namespace Udjat {

		class UDJAT_API ThreadPool {
		private:

			/// @brief Pool name
			const char *name;

			/// @brief Lock guard to prevent multiple accesses to the queue.
			std::mutex guard;

			/// @brief Task queue.
			struct Task {
				const char				* name;		///< @brief Task name.
				std::function<void()>	  callback;	///< @brief Task method.
				Task(const char *n, std::function<void()> c) : name(n), callback(c) {
				}

				Task() : name(nullptr) {};
			};
			std::queue<Task> tasks;

			static void worker(ThreadPool *pool) noexcept;

			bool pop(Task &task) noexcept;

			/// @brief Wake up one worker.
			void wakeup() noexcept;

#ifdef _WIN32

			class Controller;

			HANDLE hEvent;
			struct {
				size_t	  active		= 0;		///< @brief Number of active threads.
				size_t	  waiting		= 0;		///< @brief Número of idle threads.
			} threads;

#else
			struct {
				std::atomic<size_t>	  active;		///< @brief Number of active threads.
				std::atomic<size_t>	  waiting;		///< @brief Número of idle threads.
			} threads;

			struct {
				std::mutex m;
				std::condition_variable cv;
			} event;

#endif // _WIN32

		protected:

			struct {
				size_t threads	= 3;	///< @brief Limit the number of threads.
				size_t tasks	= 1000;	///< @brief Limit the number of tasks.
				size_t idle		= 5;	///< @brief How many seconds a thread stay idle before finish.
			} limits;

		public:

			static ThreadPool & getInstance();

			/// @brief Create a new threadpool
			/// @param name Pool name (should be an static string).
			ThreadPool(const char *name);

			~ThreadPool();

			void stop();
			void set(const XML::Node &node);

			inline void setMaxThreads(size_t maxthreads) {
				limits.threads = maxthreads;
			}

			bool empty() const noexcept {
				return (threads.active + threads.waiting) == 0;
			}

			inline operator bool() const noexcept {
				return threads.active > 0;
			}

			/// @brief Get number of active threads.
			inline size_t getActiveThreads() const noexcept {
				return threads.active;
			}

			/// @brief Get number of waiting threads.
			inline size_t getWaitingThreads() const noexcept {
				return threads.waiting;
			}

			size_t size();

			/// @brief Wait for pool cleanup.
			/// @param seconds time to wait
			bool wait(time_t seconds);

			/// @brief Wait for pool cleanup, using default timeout.
			bool wait();

			/// @brief Push a named task.
			/// @param name	Task name (Should be a static string).
			/// @param callback Task method.
			size_t push(const char *name, std::function<void()> callback);

			/// @brief Push an unnamed task.
			/// @param callback Task method.
			// size_t push(std::function<void()> callback);
			inline size_t push(std::function<void()> callback) {
				return push(__FILE__,callback);
			}

		};

	}

#endif // UDJAT_THREADPOOL_H_INCLUDED
