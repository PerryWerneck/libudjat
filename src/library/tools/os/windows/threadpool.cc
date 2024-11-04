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

 // Disable debug messages for this source.
 #ifdef DEBUG
	#undef DEBUG
 #endif // DEBUG

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/misc.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <list>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	ThreadPool & ThreadPool::getInstance() {

		class Pool : public ThreadPool {
		public:
			Pool() : ThreadPool("ThreadPool") {
				Logger::String{"Creating standard pool with ",limits.threads," threads"}.write(Logger::Debug,name);
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

#ifdef DEBUG
		cerr << name << "\tWake-Up on event " << hex << hEvent << dec << endl;
#endif // DEBUG

		if (!SetEvent(hEvent)) {
			cerr << name << "\tError '" << Win32::Exception::format("Error setting event semaphore") << endl;
			limits.threads = 0;
		}

	}

	void ThreadPool::stop() {

		wait();

		// Wait for tasks
		limits.threads = 0;

		if(getActiveThreads()) {

			debug("Waiting for ",getActiveThreads()," active threads on pool (",getWaitingThreads()," waiting)");

			for(size_t f=0; f < 1000 && getActiveThreads() > 0; f++) {

				debug(" threads.active=",threads.active," threads.waiting=",threads.waiting," limits.threads=",limits.threads);

				if(getWaitingThreads()) {
					wakeup();
				}

				Sleep(100);
			}

			{
				size_t count = getActiveThreads();
				if(count) {
					cerr << name << "\tStopping with " << count << " threads on pool" << endl;
				} else {
					cout << name << "\tStopping with no pending threads" << endl;
				}
			}

		}

	}

	size_t ThreadPool::size() {
		std::lock_guard<std::mutex> lock(this->guard);
		return tasks.size();
	}

	bool ThreadPool::wait() {
		return wait(Config::get(name,"wait-timeout",5));
	}

	bool ThreadPool::wait(time_t seconds) {

		if(size()) {

			Logger::String("Waiting for ",tasks.size()," tasks on pool").write(Logger::Trace,name);

			seconds *= (time_t) 100;
			for(time_t f=0; f < seconds && size() > 0; f++) {
				Sleep(10);
			}

			if(size()) {
				Logger::String("Timeout waiting for ",tasks.size()," tasks on pool").write(Logger::Trace,name);
			}

		}

		return size() != 0;
	}

	size_t ThreadPool::push(const char *name, std::function<void()> callback) {

		if(!limits.threads) {
			cerr << "ThreadPoool\tPool is disabled, running task '" << name << "' in foreground" << endl;
			callback();
			return tasks.size();
		}

		std::lock_guard<std::mutex> lock(this->guard);

		debug("Inserting task size=",tasks.size()," Limit=",limits.tasks);

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
			thread{worker, this}.detach();
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
		cout << pool->name << "\tMainLoop starts - ActiveThreads=" << pool->getActiveThreads() << " maxthreads=" << pool->limits.threads << endl;
#endif // DEBUG

		bool enabled = true;
		while(enabled && pool->threads.active <= pool->limits.threads) {

			Task task;

			while(pool->limits.threads && enabled) {

				if(pool->pop(task)) {

					try {

						debug("Calling '",task.name,"'");
						task.callback();
						debug("Returned from '",task.name,"'");

					} catch(const std::exception &e) {

						cerr << "ThreadPool\tTask '" << task.name << "' has failed: " << e.what() << endl;

					} catch(...) {

						cerr << "ThreadPool\tUnexpected error running task '" << task.name << "'" << endl;

					}

				} else {

					break;

				}

			}

			{
				// Wait for event
				{
					std::lock_guard<std::mutex> lock(pool->guard);
					pool->threads.waiting++;
				}
#ifdef DEBUG
				cout	<< pool->name << "\t Will wait for object "
						<< "handle=" << hex << pool->hEvent << dec
						<< " threads.active=" << pool->threads.active
						<< " threads.waiting=" << pool->threads.waiting
						<< " limits.threads=" << pool->limits.threads
						<< endl;
#endif // DEBUG
				DWORD dwWaitResult = WaitForSingleObject(pool->hEvent,(pool->limits.idle * 1000));

#ifdef DEBUG
				cout	<< pool->name << "\tWait for object returned with rc " << dwWaitResult
						<< " threads.active=" << pool->threads.active
						<< " threads.waiting=" << pool->threads.waiting
						<< " limits.threads=" << pool->limits.threads
						<< endl;
#endif // DEBUG

				{
					std::lock_guard<std::mutex> lock(pool->guard);
					pool->threads.waiting--;
				}

				switch(dwWaitResult) {
				case WAIT_OBJECT_0:
#ifdef DEBUG
					cout << pool->name << "\tWake up received" << endl;
#endif // DEBUG
					break;

				case WAIT_TIMEOUT:
#ifdef DEBUG
					cout << pool->name << "\tTimeout, stopping worker" << endl;
#endif // DEBUG
					enabled = false;
					break;

				case WAIT_ABANDONED:
					clog << pool->name << "\tEvent semaphore was abandoned" << endl;
					enabled = false;
					break;

				case WAIT_FAILED:
					cerr << pool->name << "\t" << Win32::Exception::format("Error on event semaphore");
					enabled = false;
					break;

				default:
					cerr << pool->name << "\tunexpected return " << dwWaitResult << " from WaitForSingleObject()" << endl;
					enabled = false;
				}

			}


#ifdef DEBUG
			cout << pool->name << "\tMainLoop is waking up - ActiveThreads: " << pool->getActiveThreads() << endl;
#endif // DEBUG

		}

		pool->threads.active--;

#ifdef DEBUG
		cout << pool->name << "\tMainLoop ends - ActiveThreads: " << pool->getActiveThreads() << "/" << pool->limits.threads << endl;
#endif // DEBUG

	}

 }



