/**
 * @file src/core/agent/controller.cc
 *
 * @brief Implements the root agent.
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

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Agent::Controller::Controller() : Worker(I_("agent")), Factory(I_("agent")), MainLoop::Service() {

		static const Udjat::ModuleInfo info{
			PACKAGE_NAME,								// The module name.
			"Agent Controller",							// The module description.
			PACKAGE_VERSION "." PACKAGE_RELEASE,		// The module version.
#ifdef PACKAGE_URL
			PACKAGE_URL,
#else
			"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
			PACKAGE_BUG_REPORT
#else
			""
#endif // PACKAGE_BUG_REPORT
		};

		Worker::info = &info;
		Factory::info = &info;

		cout << "agent\tStarting controller" << endl;

		MainLoop::getInstance().insert(this,1,[this](time_t now) {
			if(isActive()) {
				onTimer(now);
			}
			return true;
		});

	}

	Abstract::Agent::Controller::~Controller() {
		cout << "agent\tStopping controller" << endl;
		MainLoop::getInstance().remove(this);
	}

	void Abstract::Agent::Controller::set(std::shared_ptr<Abstract::Agent> root) {
		this->root = root;
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

	void Abstract::Agent::Controller::work(Request &request, Response &response) const {

		auto agent = find(request.c_str());

		if(!agent) {
			throw system_error(ENOENT,system_category(),"Can't find requested agent");
		}

#ifdef DEBUG
		cout << "Getting response for agent '" << agent->getName() << "'" << endl;
#endif // DEBUG

		agent->get(request,response);

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path) const {

		auto root = get();

		if(path && *path)
			return root->find(path);

		return root;

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::set_root(std::shared_ptr<Abstract::Agent> agent) {
		Abstract::Agent::Controller::getInstance().set(agent);
		return agent;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::get_root() {
		return Abstract::Agent::Controller::getInstance().get();
	}

	void Abstract::Agent::Controller::start() noexcept {

		cout << "agent\tStarting service" << endl;

		if(root) {

			try {
				root->start();
			} catch(const std::exception &e) {
				root->error("Error '{}' starting root agent",e.what());;
				return;
			}

		} else {

			clog << "agent\tStarting controller without root agent" << endl;

		}

	}

	void Abstract::Agent::Controller::stop() noexcept {

		cout << "agent\tStopping service" << endl;

		if(root) {

			try {
				root->stop();
			} catch(const std::exception &e) {
				root->error("Error '{}' stopping root agent",e.what());
			}

		} else {

			clog << "agent\tStopping controller without root agent" << endl;

		}

	}

	void Abstract::Agent::Controller::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {

		static const struct
		{
			const char *type;
			function< std::shared_ptr<Abstract::Agent>()> build;
		} builders[] = {

			{
				"integer",
				[]() {
					return make_shared<Udjat::Agent<int>>();
				}

			},
			{
				"int32",
				[]() {
					return make_shared<Udjat::Agent<int32_t>>();
				}
			},

			{
				"uint32",
				[]() {
					return make_shared<Udjat::Agent<uint32_t>>();
				}
			},

			{
				"boolean",
				[]() {
					return make_shared<Udjat::Agent<bool>>();
				}
			},

			{
				"string",
				[]() {
					return make_shared<Udjat::Agent<std::string>>();
				}
			},

			/*
			{
				"timestamp",
				[]() {
					return make_shared<Udjat::Agent<TimeStamp>>();
				}
			},
			*/

		};

		const char *type = node.attribute("type").as_string("int32");

		for(auto builder : builders) {

			if(!strcasecmp(type,builder.type)) {
				auto agent = builder.build();
				agent->load(node);
				parent.insert(agent);
				break;
			}

		}

	}

	void Abstract::Agent::Controller::onTimer(time_t now) noexcept {

		if(!root) {
			return;
		}

		// Check for updates on another thread; we'll change the
		// timer and it has a mutex lock while running the callback.
		ThreadPool::getInstance().push([this,now]() {

			time_t next = time(nullptr) + Config::Value<time_t>("agent","max-update-time",600);

			root->foreach([now,this,&next](std::shared_ptr<Agent> agent){

				// Return if no update timer.
				if(!agent->update.next)
					return;

				// If the update is in the future, adjust delay and return.
				if(agent->update.next > now) {
					next = std::min(next,agent->update.next);
					return;
				}

				if(agent->update.running) {

					agent->warning(
						"Update is active since {}",
						TimeStamp(agent->update.running).to_string()
					);

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
					next = std::min(next,agent->update.next);
				}

				// Enqueue agent update.
				ThreadPool::getInstance().push([this,agent]() {

					try {

						agent->refresh();

					} catch(const exception &e) {

						agent->failed(e,"Update failed");

					} catch(...) {

						agent->failed("Unexpected error when updating");

					}

					agent->updating(false);

				});

			});

			MainLoop::getInstance().reset(this,1,next);

		});
	}

}

