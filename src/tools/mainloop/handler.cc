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

 #include "private.h"
 #include <udjat-internals.h>

 namespace Udjat {

	void MainLoop::Handler::enable() noexcept {
		enabled = true;
		MainLoop::getInstance().wakeup();
	}

	const void * MainLoop::Handler::id() const noexcept {
		return (void *) this;
	}

	void MainLoop::Handler::disable() noexcept {
		enabled = false;
		MainLoop::getInstance().wakeup();
	}

	void MainLoop::Handler::clear() noexcept {
		fd = -1;
		enabled = false;
		MainLoop::getInstance().wakeup();
	}

	MainLoop::Handler::~Handler() {
	}

	void MainLoop::push_back(std::shared_ptr<MainLoop::Handler> handler) {

		{
			lock_guard<mutex> lock(guard);
			handlers.push_back(handler);
		}

		wakeup();

	}

	void MainLoop::remove(std::shared_ptr<Handler> handler) {

		{
			lock_guard<mutex> lock(guard);
			handlers.remove(handler);
		}

		handler->fd = -1;
		handler->disable();

	}

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

			if(!handle->enabled || handle->fd <=0) {
				handle->index = -1;
			} else {
				handle->index = nfds;
				(*fds)[nfds].fd = handle->fd;
				(*fds)[nfds].events = handle->events;
				(*fds)[nfds].revents = 0;
				nfds++;
			}

//#ifdef DEBUG
//			cout << "Handle " << handle->id() << " fd=" << handle->fd << " index=" << handle->index << " event=" << handle->events << endl;
//#endif // DEBUG

		}

		return nfds;
	}
#endif // _WIN32

 }

