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
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <private/mainloop.h>
 #include <private/misc.h>

 namespace Udjat {

	MainLoop::Handler::Handler(int f, const Event e) : fd(f), events(e) {
#ifdef DEBUG
		cout << "handler\tCreating handler " << hex << ((void *) this) << dec << endl;
#endif // DEBUG
	}

	MainLoop::Handler::~Handler() {
#ifdef DEBUG
		cout << "handler\tDestroying handler " << hex << ((void *) this) << dec << endl;
#endif // DEBUG
		disable();
	}

	bool MainLoop::Handler::enabled() const noexcept {

		MainLoop &mainloop{MainLoop::getInstance()};

		lock_guard<mutex> lock(mainloop.guard);
		for(auto handler : mainloop.handlers) {
			if(handler == this) {
				return true;
			}
		}

		return false;
	}

	bool MainLoop::Handler::enable() noexcept {

		MainLoop &mainloop{MainLoop::getInstance()};
		lock_guard<mutex> lock(mainloop.guard);

		// Is the handler enabled?
		for(auto handler : mainloop.handlers) {
			if(handler == this) {
				return false;
			}
		}

		mainloop.handlers.push_back(this);
		mainloop.wakeup();

		return true;
	}

	void MainLoop::Handler::disable() noexcept {

		MainLoop &mainloop{MainLoop::getInstance()};
		lock_guard<mutex> lock(mainloop.guard);
#ifndef _WIN32
		index = -1;
#endif // _WIN32
		mainloop.handlers.remove(this);

	}

	ssize_t MainLoop::Handler::read(void *buf, size_t count) {
		return ::read(fd,buf,count);
	}

	void MainLoop::Handler::close() {
		disable();
		::close(fd);
	}

	void MainLoop::Handler::set(int fd) {

		if(this->fd != -1) {
			throw system_error(EBUSY,system_category(),"Handler already have a file descriptor");
		}

		this->fd = fd;

	}

	void MainLoop::Handler::set(const Event events) {
		this->events = events;
		if(enabled()) {
			MainLoop::getInstance().wakeup();
		}
	}

	/*
	void MainLoop::Handler::clear() noexcept {
		fd = -1;
		enabled = false;
		MainLoop::getInstance().wakeup();
	}
	*/

	/*
	void MainLoop::push_back(std::shared_ptr<MainLoop::Handler> handler) {

		{
			lock_guard<mutex> lock(guard);
			handlers.push_back(handler);
		}

		wakeup();

	}
	*/

	/*
	void MainLoop::remove(std::shared_ptr<Handler> handler) {

#ifdef DEBUG
		cout << "handler\tAARemoving handler " << hex << ((void *) handler.get()) << dec << " with " << handler.use_count() << " pending instance(s)" << endl;
#endif // DEBUG

		{
			lock_guard<mutex> lock(guard);
			handlers.remove(handler);
		}

#ifdef DEBUG
		cout << "handler\tRemoving handler " << hex << ((void *) handler.get()) << dec << " with " << handler.use_count() << " pending instance(s)" << endl;
#endif // DEBUG

		handler->fd = -1;
		handler->disable();

	}
	*/

	/*
	std::shared_ptr<MainLoop::Handler> MainLoop::insert(const void *id, int fd, const Event event, const function<bool(const Event event)> call) {

		class CallHandler : public MainLoop::Handler {
		private:
			const void * identifier;
			const function<bool(const Event event)> callback;

		protected:
			bool call(const Event event) override {
				return callback(event);
			}

		public:
			CallHandler(const void *i, int fd, const Event event, const function<bool(const Event event)> c) : Handler(fd,event), identifier(i), callback(c) {
			}

			const void * id() const noexcept override {
				return identifier;
			}

		};

		std::shared_ptr<Handler> handler = make_shared<CallHandler>(id,fd,event,call);
		push_back(handler);
		return handler;

	}
	*/


#ifndef _WIN32
	nfds_t MainLoop::getHandlers(struct pollfd **fds, nfds_t *length) {

		lock_guard<mutex> lock(guard);

		if(*length <= handlers.size()) {
			*length = handlers.size()+1;	// 1 extra for the eventfd.
			*fds = (struct pollfd *) realloc(*fds, sizeof(struct pollfd) * *length);
		}

		// Get waiting sockets.
		nfds_t nfds = 0;
		for(auto handle : handlers) {

			handle->index = nfds;
			(*fds)[nfds].fd = handle->fd;
			(*fds)[nfds].events = handle->events;
			(*fds)[nfds].revents = 0;
			nfds++;

		}

		return nfds;
	}
#endif // _WIN32

 }

