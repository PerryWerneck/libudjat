/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	ThreadPool & ThreadPool::getInstance() {
		class Pool : public ThreadPool {
		public:
			Pool() : ThreadPool("ThreadPool") {
				Logger(name).info("Creating standard pool with {} threads",limits.threads);
			}
		};

		static Pool threadpool;
		return threadpool;
	}

	ThreadPool::ThreadPool(const char *n) : name(n)  {

		threads.active = threads.waiting = 0;

		try {

			hEvent = CreateEvent(
				NULL,	// lpEventAttributes,
				FALSE,	// bManualReset,
				FALSE,	// bInitialState,
				NULL	// lpName
			);

			if(!hEvent) {
				throw Win32::Exception("Can't create event semaphore");
			}

			limits.threads	= Config::get(name,"max-threads",limits.threads);
			limits.tasks	= Config::get(name,"max-tasks",limits.tasks);
			limits.idle		= Config::get(name,"max-idle",limits.idle);

		} catch(const std::exception &e) {

			cerr << name << "\tError '" << e.what() << "' loading threadpool settings" << endl;

		}

	}

	ThreadPool::~ThreadPool() {
		stop();
		CloseHandle(hEvent);
	}

	void ThreadPool::wakeup() noexcept {

		if (!SetEvent(hEvent)) {
			cerr << name << "\tError '" << Win32::Exception::format("Error setting event semaphore") << endl;
			limits.threads = 0;
		}

	}

	/*
	void ThreadPool::set(const pugi::xml_node &node) {

		limits.threads	= node.attribute("max-threads").as_uint(limits.threads);
		limits.tasks	= node.attribute("max-tasks").as_uint(limits.tasks);
		limits.idle		= node.attribute("max-idle").as_uint(limits.idle);

	}
	*/

	void ThreadPool::stop() {

		Logger logger(name);

		wait();

		// Wait for tasks
		limits.threads = 0;

		if(threads.active.load()) {

			logger.info("Waiting for {} threads on pool",threads.active.load());

#ifdef DEBUG
			cout << "threads.waiting.load()=" << threads.waiting.load() << endl;
#endif // DEBUG

			for(size_t f=0; f < 1000 && threads.active.load() > 0; f++) {

				if(threads.waiting.load()) {
					wakeup();
				}

				Sleep(100);
			}

			logger.error("Stopping with {} threads on pool",threads.active.load());

		}

	}

	size_t ThreadPool::size() {
		std::lock_guard<std::mutex> lock(this->guard);
		return tasks.size();
	}

	void ThreadPool::wait() {

		Logger logger(name);

		if(size()) {

			logger.warning("Waiting for {} tasks on pool",tasks.size());

			for(size_t f=0; f < 100000 && size() > 0; f++) {
				Sleep(100);
			}

			if(size()) {
				logger.error("Timeout waiting for {} tasks on pool",tasks.size());
			}

		}

	}

	size_t ThreadPool::push(std::function<void()> callback) {
		return push(this->name,callback);
	}

	size_t ThreadPool::push(const char *name, std::function<void()> callback) {

		std::lock_guard<std::mutex> lock(this->guard);

#ifdef DEBUG
		cout << "Inserting task size=" << tasks.size() << " Limit=" << limits.tasks << endl;
#endif // DEBUG

		if(limits.tasks && tasks.size() >= limits.tasks) {
			throw std::runtime_error(
				string{"Can't add new task, the queue has reached the limit of "}
					+ to_string(limits.tasks)
					+ " tasks"
			);
		}

		tasks.emplace(name,callback);

		if(threads.waiting) {
			wakeup();
		} else if(threads.active < limits.threads) {
			std::thread(worker, this).detach();
		}

		return tasks.size();
	}

	bool ThreadPool::pop(Task &task) noexcept {

		std::lock_guard<std::mutex> lock(this->guard);

		if(tasks.empty()) {
			return false;
		}

		task = tasks.front();

		tasks.pop();

		return true;

	}

	void ThreadPool::worker(ThreadPool *pool) noexcept {

		pool->threads.active++;

#ifdef DEBUG
		cout << pool->name << "\tMainLoop starts - ActiveThreads: " << pool->threads.active.load() << "/" << pool->limits.threads << endl;
#endif // DEBUG

		while(pool->threads.active <= pool->limits.threads) {

			Task task;

			while(pool->limits.threads) {

				if(pool->pop(task)) {

					try {

#ifdef DEBUG
						cout << pool->name << "\tRunning worker '" << task.name << "'" << endl;
#endif // DEBUG

						task.callback();

#ifdef DEBUG
						cout << pool->name << "\tWorker '" << task.name << "' is complete" << endl;
#endif // DEBUG

						if(task.name && task.name != pool->name) {
							pthread_setname_np(pthread_self(),pool->name);
						}

					} catch(const std::exception &e) {

						cerr << task.name << "\t" << e.what() << endl;

					} catch(...) {

						cerr << task.name << "\tUnexpected error running delayed task" << endl;

					}

				} else {

					break;

				}

			}

#ifdef DEBUG
			cout	<< pool->name
					<< "\tMainLoop is waiting - ActiveThreads: "
					<< pool->threads.active.load() << "/" << pool->limits.threads
					<< " delay=" << pool->limits.idle << endl;
#endif // DEBUG

			{
				// Wait for event
				pool->threads.waiting++;
				DWORD dwWaitResult = WaitForSingleObject(pool->hEvent,(pool->limits.idle * 1000));
				pool->threads.waiting--;

				if(dwWaitResult == WAIT_TIMEOUT) {
#ifdef DEBUG
					cout << pool->name << "\tTimeout, stopping worker" << endl;
#endif // DEBUG
					break;
				} else if(dwWaitResult == WAIT_ABANDONED) {
					clog << pool->name << "\tEvent semaphore was abandoned" << endl;
					break;
				} else if(dwWaitResult == WAIT_FAILED) {
					cerr << pool->name << "\t" << Win32::Exception::format("Error on event semaphore");
					break;
				}

			}


#ifdef DEBUG
			cout << pool->name << "\tMainLoop is waking up - ActiveThreads: " << pool->threads.active.load() << endl;
#endif // DEBUG

		}

		pool->threads.active--;

#ifdef DEBUG
		cout << pool->name << "\tMainLoop ends - ActiveThreads: " << pool->threads.active.load() << "/" << pool->limits.threads << endl;
#endif // DEBUG

	}

 }



