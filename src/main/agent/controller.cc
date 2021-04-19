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

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Agent::Controller::Controller() : Worker(I_("agent")), Factory(I_("agent")), Module(I_("agent")) {

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
			PACKAGE_BUG_REPORT,
#else
			"",
#endif // PACKAGE_BUG_REPORT

			nullptr
		};

		Worker::info = &info;
		Factory::info = &info;
		Module::info = &info;

	}

	void Abstract::Agent::Controller::set(std::shared_ptr<Abstract::Agent> root) {
		this->root = root;
		this->active = this->root.get();
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

	void Abstract::Agent::Controller::work(const Request &request, Response &response) const {

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

	void Abstract::Agent::Controller::start() {
		if(root) {
			root->start();
		}
	}

	void Abstract::Agent::Controller::stop() {
		if(root) {
			root->stop();
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

}

