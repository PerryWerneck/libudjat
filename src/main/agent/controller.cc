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
 #include <private/agent.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file.h>
 #include <unistd.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo{ N_( "Agent controller" ) };

	Abstract::Agent::Controller::Controller() : Worker("agent",moduleinfo), Factory("agent",moduleinfo), MainLoop::Service("agents",moduleinfo) {

		cout << "agent\tInitializing controller" << endl;

	}

	Abstract::Agent::Controller::~Controller() {
		cout << "agent\tDeinitializing controller" << endl;
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

			try {
				root->start();

				// Setup next update on all children.
				root->for_each([](std::shared_ptr<Agent> agent) {
					if(agent->update.timer && !agent->update.next) {
						agent->update.next = time(0) + agent->update.timer;
					}
				});

			} catch(const std::exception &e) {
				cerr << root->name() << "\tError '" << e.what() << "' starting root agent" << endl;
				return;
			}

		} else {

			clog << "agent\tStarting controller without root agent" << endl;

		}

		cout << "agent\tStarting controller" << endl;



		MainLoop::Timer::reset(1000);
		MainLoop::Timer::enable();

	}

	void Abstract::Agent::Controller::stop() noexcept {

		cout << "agent\tStopping controller" << endl;

		MainLoop::Timer::disable();

		if(root) {

			try {
				root->stop();
			} catch(const std::exception &e) {
				root->error() << "Error '" << e.what() << "' stopping root agent" << endl;
			} catch(...) {
				root->error() << "Unexpected error stopping root agent" << endl;
			}

			root.reset();

			debug("Waiting for tasks (agent)");
			ThreadPool::getInstance().wait();
			debug("Wait for tasks complete");

		}

	}

	void Abstract::Agent::Controller::update_agents() {

		time_t now{time(0)};
		time_t next{now+Config::Value<time_t>("agent","min-update-time",600)};

		std::vector<std::shared_ptr<Agent>> updatelist;

		root->for_each([now,this,&next,&updatelist](std::shared_ptr<Agent> agent) {

			// Ignore agents without 'next' or with forwarded state.
			if(!agent->update.next || agent->current_state.forwarded()) {
				return;
			}

			// Do the agent requires an update?
			if(agent->update.next <= now) {

				lock_guard<std::recursive_mutex> lock(agent->guard);
				if(agent->update.running) {

					//
					// Agent still updating.
					//
					agent->warning() << "Update is active since " << TimeStamp(agent->update.running) << endl;
					agent->update.next = now + 60;
					next = std::min(next,agent->update.next);

				} else {

					//
					// Queue agent update.
					//
					debug("Agent='",agent->name(),"' is expired by ", (now - agent->update.next)," seconds, adding to the update list");
					agent->update.running = time(0);
					updatelist.push_back(agent);

					if(agent->update.timer) {
						agent->update.next = now + agent->update.timer;
						next = std::min(next,agent->update.next);
					} else {
						agent->update.next = 0;
					}

				}

			} else {
				debug("Agent='",agent->name(),"' update set to '",TimeStamp(agent->update.next));
				next = std::min(next,agent->update.next);
			}
		});

		//
		// Enqueue agent updates
		//
		debug(updatelist.size()," agent(s) to update, next update will be ",TimeStamp(next));

		if(now < next) {
			MainLoop::Timer::reset((next-now) * 1000);
		} else {
			MainLoop::Timer::reset(1000);
		}

		for(auto agent : updatelist) {

			agent->push([](std::shared_ptr<Agent> agent){

				try {

#ifdef DEBUG
					agent->info() << "Scheduled update begins" << endl;
#endif // DEBUG
					agent->notify(UPDATE_TIMER);
					agent->refresh(false);

#ifdef DEBUG
					agent->info() << "Scheduled update complete" << endl;
#endif // DEBUG

				} catch(const exception &e) {

					agent->failed("Agent update failed",e);

				} catch(...) {

					agent->failed("Unexpected error when updating");

				}

				{
					lock_guard<std::recursive_mutex> lock(agent->guard);
					agent->update.running = 0;
				}

			});

		}
	}

	void Abstract::Agent::Controller::on_timer() {

		if(!root) {
			debug("No root agent!!");
			return;
		}

		ThreadPool::getInstance().push("agent-updates",[this]() {

			{
				lock_guard<std::recursive_mutex> lock(Abstract::Agent::guard);
				if(updating) {
					cerr << "agents\tUpdating since " << TimeStamp(updating) << endl;
					return;
				}
				updating = time(0);
			}

			try {

				update_agents();

			} catch(const std::exception &e) {

				cerr << "Error updating agents: " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error updating agents" << endl;

			}

			{
				lock_guard<std::recursive_mutex> lock(Abstract::Agent::guard);
				updating = 0;
			}

		});


	}

}

