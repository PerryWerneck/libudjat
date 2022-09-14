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

/**
 * @file src/main/agent/controller.cc
 *
 * @brief Implements the agent controller.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include "private.h"
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file.h>
 #include <unistd.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo{ "Agent controller" };

	Abstract::Agent::Controller::Controller() : Worker("agent",moduleinfo), Factory("agent",moduleinfo), MainLoop::Service("agents",moduleinfo) {

		cout << "agent\tStarting controller" << endl;

		MainLoop::getInstance().insert(this,1000,[this]() {
			if(isActive()) {
				onTimer(time(0));
			} else {
				cout << "agent\tAgent controller is not active" << endl;
			}
			return true;
		});

	}

	Abstract::Agent::Controller::~Controller() {
		cout << "agent\tStopping controller" << endl;
		MainLoop::getInstance().remove(this);
	}

	void Abstract::Agent::Controller::set(std::shared_ptr<Abstract::Agent> root) {

		if(root && root->parent) {
			throw system_error(EINVAL,system_category(),"Child agent cant be set as root");
		}

		if(!root) {

			if(this->root) {
				cout << "agent\tRemoving root agent '"
						<< this->root->name()
						<< "' (" << hex << ((void *) this->root.get()) << dec << ")"
						<< endl;
				this->root.reset();
			}
			return;
		}

		this->root = root;

		cout << "agent\tAgent '"
				<< this->root->name()
				<< "' (" << hex << ((void *) root.get() ) << dec << ") is the new root" << endl;

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::get() const {

		if(this->root)
			return this->root;

		throw runtime_error("Agent subsystem is inactive");

	}

	Abstract::Agent::Controller & Abstract::Agent::Controller::getInstance() {
		static Controller controller;
		return controller;
	}

	bool Abstract::Agent::Controller::get(Request &request, Response &response) const {

#ifdef DEBUG
		cout << "Finding agent '" << request.getPath() << "'" << endl;
#endif // DEBUG

		auto agent = find(request.getPath());

		if(!agent) {
			throw system_error(ENOENT,system_category(),string{"No agent on '"} + request.getPath() + "'");
		}

		agent->head(response);
		agent->get(request,response);

		return true;
	}

	bool Abstract::Agent::Controller::head(Request &request, Response &response) const {

		auto agent = find(request.getPath());

		if(!agent) {
			throw system_error(ENOENT,system_category(),string{"No agent on '"} + request.getPath() + "'");
		}

		agent->head(response);

		return true;
	}

	bool Abstract::Agent::Controller::work(Request &request, Report &response) const {

		auto agent = find(request.getPath());

		if(!agent) {
			throw system_error(ENOENT,system_category(),string{"No agent on '"} + request.getPath() + "'");
		}

		agent->head(response);
		agent->get(request,response);

		return true;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path) const {

		auto root = get();

		if(path && *path)
			return root->find(path);

		return root;

	}

	void Abstract::Agent::deinit() {
		Abstract::Agent::Controller::getInstance().set(std::shared_ptr<Abstract::Agent>());
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::root() {
		return Abstract::Agent::Controller::getInstance().get();
	}

	void Abstract::Agent::Controller::start() noexcept {

		if(root) {

			cout << "agent\tStarting controller" << endl;

			try {
				root->start();
			} catch(const std::exception &e) {
				cerr << root->name() << "\tError '" << e.what() << "' starting root agent" << endl;
				return;
			}

		} else {

			clog << "agent\tStarting controller without root agent" << endl;

		}

	}

	void Abstract::Agent::Controller::stop() noexcept {

		if(root) {

			try {
				root->stop();
			} catch(const std::exception &e) {
				root->error() << "Error '" << e.what() << "' stopping root agent" << endl;
			} catch(...) {
				root->error() << "Unexpected error stopping root agent" << endl;
			}

			root.reset();

#ifdef DEBUG
			cout << "agent\t*** Waiting for tasks " << __FILE__ << "(" << __LINE__ << ")" << endl;
#endif // DEBUG
			ThreadPool::getInstance().wait();
#ifdef DEBUG
			cout << "agent\t*** Wait for tasks complete" << endl;
#endif // DEBUG

		}

	}

	void Abstract::Agent::Controller::onTimer(time_t now) noexcept {

		if(!root) {
			return;
		}

		if(root->update.running) {
			root->error() << "Updating since " << TimeStamp(root->update.running) << endl;
			return;
		}

		root->updating(true);

//#ifdef DEBUG
//		cout << "Checking for updates" << endl;
//#endif // DEBUG

		// Check for updates on another thread; we'll change the
		// timer and it has a mutex lock while running the callback.
		ThreadPool::getInstance().push("agent-updates",[this,now]() {

			time_t next = time(nullptr) + Config::Value<time_t>("agent","max-update-time",600);

			root->for_each([now,this,&next](std::shared_ptr<Agent> agent) {

				// Return if no update timer.
				if(!agent->update.next)
					return;

#ifdef DEBUG
				cout << "TIMER=" << agent->update.timer << " Next=" << TimeStamp(agent->update.next) << endl;
#endif // DEBUG

				// If the update is in the future, adjust delay and return.
				if(agent->update.next > now) {
					next = std::min(next,agent->update.next);
					return;
				}

				if(agent->update.running) {

					clog << agent->name() << "\tUpdate is active since " << TimeStamp(agent->update.running) << endl;

					if(agent->update.timer) {
						agent->update.next = now + agent->update.timer;
					} else {
						agent->update.next = now + 60;
					}

					return;
				}

				// Agent requires update.
				agent->updating(true);

				if(agent->update.timer) {

					agent->update.next = time(0) + agent->update.timer;
#ifdef DEBUG
					agent->info() << "**** Next update scheduled to " << TimeStamp(agent->update.next) << " (" << agent->update.timer << " seconds)" << endl;
#endif // DEBUG
					next = std::min(next,agent->update.next);

				} else {

					// No timer and updated was triggered, reset next update time.
					agent->update.next = 0;

				}

				// Enqueue agent update.
				agent->push([this,agent]() {

					try {

						agent->refresh(false);

					} catch(const exception &e) {

						agent->failed("Agent update failed",e);

					} catch(...) {

						agent->failed("Unexpected error when updating");

					}

					agent->updating(false);

				});

			});

			root->updating(false);

		});

	}

}

