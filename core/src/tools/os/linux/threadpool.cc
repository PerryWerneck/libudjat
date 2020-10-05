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
 #include <unistd.h>
 #include <semaphore.h>
 #include <cstring>
 #include <chrono>
 #include <unistd.h>
 #include <iostream>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	ThreadPool & ThreadPool::getInstance() {
		static ThreadPool threadpool;
		return threadpool;
	}

	ThreadPool::ThreadPool() {

		threads.active	= 0;
		threads.waiting	= 0;

		limits.threads	= 3;
		limits.tasks	= 1000;
		limits.idle		= 5;

	}

	ThreadPool::ThreadPool(const pugi::xml_node &node) : ThreadPool() {

		set(node);

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

		if(tasks.size()) {
			cout << "Stopping thread pool with " << tasks.size() << " pending tasks" << endl;
		}

		// Wait for tasks
		limits.threads = 0;

		if(threads.active) {

			cout << "Waiting for " << threads.active.load() << " threads on pool" << endl;

			for(size_t f=0; f < 1000 && (threads.active || threads.waiting); f++) {

				if(threads.waiting.load()) {
					wakeup();
				}

				usleep(100);
			}

		}

		if(threads.active) {
			cerr << "Timeout waiting for " << threads.active.load() << " threads" << endl;
		}

	}

	size_t ThreadPool::push(std::function<void()> callback) {

		std::lock_guard<std::mutex> lock(this->guard);

		if(limits.tasks && tasks.size() >= limits.tasks) {
			throw std::runtime_error("Can't add new task, the queue has reached the limit");
		}

		tasks.push(callback);

		if(threads.waiting) {
			wakeup();
		} else if(threads.active < limits.threads) {
			std::thread(worker, this).detach();
		}

		return tasks.size();
	}

	bool ThreadPool::pop(std::function<void()> &cbk) noexcept {

		std::lock_guard<std::mutex> lock(this->guard);

		if(tasks.empty()) {
			return false;
		}

		cbk = tasks.front();

		tasks.pop();

		return true;

	}

	void ThreadPool::worker(ThreadPool *pool) noexcept {

		struct timespec   ts;

		memset(&ts,0,sizeof(ts));

		pool->threads.active++;

#ifdef DEBUG
		cout << "MainLoop starts (ActiveThreads: " << pool->threads.active.load() << "/" << endl;
#endif // DEBUG

		while(pool->threads.active <= pool->limits.threads) {

			std::function<void()> cbk;

			while(pool->limits.threads) {

				if(pool->pop(cbk)) {

					try {

						cbk();

					} catch(const std::exception &e) {

						cerr << e.what() << endl;

					} catch(...) {

						cerr << "Unexpected error when running background task" << endl;

					}

				} else {

					break;

				}

			}

#ifdef DEBUG
			cout	<< "MainLoop is waiting (ActiveThreads: "
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
				cout << "Timeout waiting for tasks" << endl;
#endif // DEBUG
				break;
			}

#ifdef DEBUG
			cout << "MainLoop is waking up (ActiveThreads: " << pool->threads.active.load() << ")" << endl;
#endif // DEBUG

		}

		pool->threads.active--;

#ifdef DEBUG
		cout << "MainLoop ends (ActiveThreads: " << pool->threads.active.load() << "/" << pool->limits.threads << endl;
#endif // DEBUG

	}

 }



