/**
 *
 * Copyright (C) <2017> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @file
 *
 * @brief Implementa objeto para o pool de threads.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 * $URL: http://suportelinux.df.bb.com.br/svn/suporte/aplicativos/common-components/cpp/src/components/core/linux/threadpool.cc $
 * $Revision: 40262 $ $Author: c1103788 $
 *
 */

 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>
 #include <semaphore.h>
 #include <cstring>
 #include <chrono>
 #include <unistd.h>
 #include <iostream>
 #include <pthread.h>
 #include <pthread.h>

 #ifdef DEBUG
	#undef DEBUG
 #endif // DEBUG

 #define THREAD_ID hex << ((unsigned long long) pthread_self()) << dec

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

			limits.threads	= Config::get(name,"max-threads",limits.threads);
			limits.tasks	= Config::get(name,"max-tasks",limits.tasks);
			limits.idle		= Config::get(name,"max-idle",limits.idle);

		} catch(const std::exception &e) {

			cerr << name << "\tError '" << e.what() << "' loading threadpool settings" << endl;

		}

	}

	void ThreadPool::wakeup() noexcept {
		event.cv.notify_one();
	}

	ThreadPool::~ThreadPool() {
		stop();
	}

	void ThreadPool::set(const pugi::xml_node &node) {

		limits.threads	= node.attribute("max-threads").as_uint(limits.threads);
		limits.tasks	= node.attribute("max-tasks").as_uint(limits.tasks);
		limits.idle		= node.attribute("max-idle").as_uint(limits.idle);

	}

	void ThreadPool::stop() {

		Logger logger(name);

		wait();

		// Wait for tasks
		limits.threads = 0;

		if(threads.active.load()) {

			clog << "threadpool\tWaiting for " << threads.active.load() << " thread(s)" << endl;

			for(size_t f=0; f < 10000 && threads.active.load() > 0; f++) {

				if(threads.waiting.load()) {
					wakeup();
				}

				usleep(20000);
			}

			{
				size_t count = threads.active.load();
				if(count) {
					cerr << "threadpool\tStopping with " << count << " threads on pool" << endl;
				} else {
					cout << "threadpool\tStopping with no pending threads" << endl;
				}
			}

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
				usleep(100);
			}

			if(size()) {
				logger.error("Timeout waiting for {} tasks on pool",tasks.size());
			}

		}

	}

	size_t ThreadPool::push(const char *name, std::function<void()> callback) {

		std::lock_guard<std::mutex> lock(this->guard);

//		cout << "Inserting task size=" << tasks.size() << " Limit=" << limits.tasks << endl;

		if(limits.tasks && tasks.size() >= limits.tasks) {
			string message{"Can't add new task, the queue has reached the limit of "};
			message += to_string(limits.tasks);
			message += " tasks";
			throw std::runtime_error(message);
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

		struct timespec   ts;

		pthread_setname_np(pthread_self(),"poolworker");
		memset(&ts,0,sizeof(ts));

		pool->threads.active++;

#ifdef DEBUG
		cout << pool->name << "\tMainLoop(" << THREAD_ID << ") starts - ActiveThreads: " << pool->threads.active.load() << "/" << pool->limits.threads << endl;
#endif // DEBUG

		while(pool->threads.active <= pool->limits.threads) {


			while(pool->limits.threads) {

				Task task;
				if(pool->pop(task)) {

					try {

						if(task.name && task.name != pool->name) {
							pthread_setname_np(pthread_self(),task.name);
						}

#ifdef DEBUG
						cout << pool->name << "\tRunning worker '" << task.name << "' on MainLoop(" << THREAD_ID << ")" << endl;
#endif // DEBUG

						task.callback();

#ifdef DEBUG
						cout << pool->name << "\tWorker '" << task.name << "' is complete on MainLoop(" << THREAD_ID << ")" << endl;
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
					<< "\tMainLoop(" << THREAD_ID << ") is waiting - ActiveThreads: "
					<< pool->threads.active.load() << "/" << pool->limits.threads
					<< " delay=" << pool->limits.idle << endl;
#endif // DEBUG

			std::unique_lock<std::mutex> lk(pool->event.m);
			std::chrono::seconds wait(pool->limits.idle);

			pool->threads.waiting++;
			auto rc = pool->event.cv.wait_for(lk,wait);
			pool->threads.waiting--;

			if(rc == std::cv_status::timeout) {
#ifdef DEBUG
				cout << pool->name << "\tTimeout waiting for tasks on MainLoop(" << THREAD_ID << ")" << endl;
#endif // DEBUG
				break;
			}

#ifdef DEBUG
			cout << pool->name << "\tMainLoop(" << THREAD_ID << ") is waking up - ActiveThreads: " << pool->threads.active.load() << endl;
#endif // DEBUG

		}

		pool->threads.active--;

#ifdef DEBUG
		cout << pool->name << "\tMainLoop(" << THREAD_ID << ") ends - ActiveThreads: " << pool->threads.active.load() << "/" << pool->limits.threads << endl;
#endif // DEBUG

	}

 }



