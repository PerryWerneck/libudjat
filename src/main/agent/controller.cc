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
 #include <udjat/agent/abstract.h>
 #include <unistd.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo{ N_( "Agent controller" ) };

	Abstract::Agent::Controller::Controller() : Worker("agent",moduleinfo), Service("agents",moduleinfo) {
		Logger::String{
			"Initializing controller"
		}.trace("agent");
	}

	Abstract::Agent::Controller::~Controller() {
		Logger::String{
			"Deinitializing controller"
		}.trace("agent");
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

		Logger::String{
			"Agent ",
			std::to_string((unsigned long long) ((void *) root.get())),
			" was promoted to root"
		}.trace(root->name());

//		this->root->info()
//				<< "Agent " << hex << ((void *) root.get() ) << dec << " was promoted to root" << endl;

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::get() const {

		if(this->root)
			return this->root;

		throw runtime_error(_("Core/Module subsystem failed to initialize"));

	}

	bool Abstract::Agent::Controller::get(Request &request, Udjat::Response::Value &response) const {

		return root->getProperties(request.path(),response);

	}

	bool Abstract::Agent::Controller::get(Request &request, Udjat::Response::Table &response) const {

		debug("-[ GET('",request.path(),"',table) ]----------------------");

		auto agent = find(request.path());
		if(!agent){
			throw std::system_error(ENOENT,std::system_category());
		}

		agent->get(response);
		return true;
	}

	/*
	bool Abstract::Agent::Controller::head(Request &request, Udjat::Response::Value &response) const {

		debug("-[ HEAD(",request.path(),") ]----------------------");

		// Get cache info.
		if(!head(get().get(),request.path(),response)) {
			throw std::system_error(ENOENT,std::system_category());
		}

		return true;
	}

	bool Abstract::Agent::Controller::get(Request &request, Udjat::Response::Value &response) const {

		debug("-[ GET(",request.path(),") ]----------------------");


		// Get properties.

		auto agent = find(request.path());
		if(!agent){
			debug("Cant find '",request.path());
			throw std::system_error(ENOENT,std::system_category());
		}

		agent->get(request,response);

		return true;

	}

	bool Abstract::Agent::Controller::get(Request &request, Udjat::Response::Table &response) const {

		debug("-[ GET-ARRAY(",request.path(),") ]----------------------");

		// Get properties.
		auto agent = find(request.path());
		if(!agent){
			throw std::system_error(ENOENT,std::system_category());
		}

		agent->get(request,response);
		return true;

	}
	*/

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path, bool required) const {

		auto root = get();

		if(path && *path)
			return root->find(path,required);

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

		Logger::String{
			"Starting controller"
		}.trace("agent");

		MainLoop::Timer::reset(1000);
		MainLoop::Timer::enable();

	}

	void Abstract::Agent::Controller::stop() noexcept {

		Logger::String{
			"Stopping controller"
		}.trace("agent");

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
				debug(
					"Agent='",agent->name(),"' will not update. Next=",agent->update.next,
					" Forwarded=",(agent->current_state.forwarded() ? "Yes" : "No"),
					" (",agent->current_state.selected->summary(),")"
				);
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
				next = std::min(next,agent->update.next);
				debug(
					"Agent='",agent->name(),
					"' update set to '",TimeStamp(agent->update.next),
					", global update set to ",TimeStamp(next)
				);
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

					debug("Scheduled update of '",agent->name(),"' begin");

					agent->notify(UPDATE_TIMER);

					if(agent->refresh(false)) {
						debug("Agent was changed");
						agent->updated(true);
					} else {
						agent->updated(false);
					}

					debug("Scheduled update of '",agent->name(),"' complete");

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
				time_t now = time(0);

				lock_guard<std::recursive_mutex> lock(Abstract::Agent::guard);
				if(updating) {
					if(updating < now) {
						cerr << "agents\tUpdating since " << TimeStamp(updating) << endl;
					}
					reset(500);
					return;
				}
				updating = now;
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

