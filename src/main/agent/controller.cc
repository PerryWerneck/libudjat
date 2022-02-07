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
 #include <udjat/tools/file.h>
 #include <unistd.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo {
		PACKAGE_NAME,									// The module name.
		"Agent controller",			 					// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	Abstract::Agent::Controller::Controller() : Worker("agent",&moduleinfo), Factory("agent",&moduleinfo), MainLoop::Service(&moduleinfo) {

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

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path) const {

		auto root = get();

		if(path && *path)
			return root->find(path);

		return root;

	}

	/*
	std::shared_ptr<Abstract::Agent> Abstract::Agent::init(std::shared_ptr<Abstract::Agent> agent) {
		Abstract::Agent::Controller::getInstance().set(agent);
		return agent;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::init() {
		return init(getDefaultRootAgent());
	}
	*/

	/*
	std::shared_ptr<Abstract::Agent> Abstract::Agent::init(const char *path) {

		Abstract::Agent::Controller &controller = Abstract::Agent::Controller::getInstance();
		auto root = getDefaultRootAgent();

		File::List(path).forEach([root](const char *filename){

			cout << endl << "agent\tLoading '" << filename << "'" << endl;
			pugi::xml_document doc;
			doc.load_file(filename);
			root->load(doc);

		});

		controller.set(root);

		return root;

	}
	*/

	void Abstract::Agent::deinit() {
		Abstract::Agent::Controller::getInstance().set(std::shared_ptr<Abstract::Agent>());
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::get_root() {
		return root();
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::root() {
		return Abstract::Agent::Controller::getInstance().get();
	}

	void Abstract::Agent::Controller::start() noexcept {

		if(root) {

			clog << "agent\tStarting controller" << endl;

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
				cerr << root->name() << "\tError '" << e.what() << "' stopping root agent" << endl;
			}

		} else {

			clog << "agent\tStopping controller without root agent" << endl;

		}

	}

	bool Abstract::Agent::Controller::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {

		static const struct
		{
			const char *type;
			function< std::shared_ptr<Abstract::Agent>(const pugi::xml_node &node)> build;
		} builders[] = {

			{
				"integer",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<int>>(node);
				}

			},
			{
				"int32",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<int32_t>>(node);
				}
			},

			{
				"uint32",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<uint32_t>>(node);
				}
			},

			{
				"boolean",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<bool>>(node);
				}
			},

			{
				"string",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<std::string>>(node);
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
				auto agent = builder.build(node);
				agent->load(node);
				parent.insert(agent);
				return true;
			}

		}

		// TODO: Call factory based on agent type.

		return false;
	}

	void Abstract::Agent::Controller::onTimer(time_t now) noexcept {

		if(!root) {
			return;
		}

//#ifdef DEBUG
//		cout << "Checking for updates" << endl;
//#endif // DEBUG

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
					next = std::min(next,agent->update.next);
				}

				// Enqueue agent update.
				ThreadPool::getInstance().push([this,agent]() {

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

		});

	}

}

