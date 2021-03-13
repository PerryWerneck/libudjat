
#include "private.h"
#include <unordered_map>

using namespace std;

namespace Udjat {

	class Abstract::Agent::Factory::Controller {
	private:

		static recursive_mutex guard;

		// Hash method
		class Hash {
		public:
			inline size_t operator() (const char * str) const {
				// https://stackoverflow.com/questions/7666509/hash-function-for-string
				size_t value = 5381;

				for(const char *ptr = str; *ptr; ptr++) {
					value = ((value << 5) + value) + tolower(*ptr);
				}

				return value;
			}
		};

		// Equal method
		class Equal {
		public:
			inline bool operator() (const char *a, const char *b) const {
				return strcasecmp(a,b) == 0;
			}
		};

		std::unordered_map<const char *, const Abstract::Agent::Factory *, Hash, Equal> methods;

		Controller() {
		}

	public:
		static Controller & getInstance() {
			lock_guard<recursive_mutex> lock(Abstract::Agent::Factory::Controller::guard);
			static Controller instance;
			return instance;
		}

		void insert(const Factory *factory) {
			lock_guard<recursive_mutex> lock(guard);
			methods.insert(make_pair(factory->c_str(),factory));
		}

		void remove(const Factory *factory) {
			lock_guard<recursive_mutex> lock(guard);

			auto entry = methods.find(factory->c_str());
			if(entry == methods.end())
				return;

			if(entry->second != factory)
				return;

			methods.erase(entry);

		}

		bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const {

			auto entry = methods.find(name);

			if(entry == methods.end())
				return false;

			entry->second->parse(parent,node);

			return true;
		}


	};

	recursive_mutex Abstract::Agent::Factory::Controller::guard;

	Abstract::Agent::Factory::Factory(const Quark &n) : name(n) {
		Controller::getInstance().insert(this);
	}

	Abstract::Agent::Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	bool Abstract::Agent::Factory::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

	/*
	recursive_mutex Factory::Controller::guard;

	Factory::Controller::Controller() {

		insert(I_("integer"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int>>(&parent,node);
		});

		insert(I_("int32"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int32_t>>(&parent,node);
		});

		insert(I_("uint32"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<uint32_t>>(&parent,node);
		});

		insert(I_("boolean"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<bool>>(&parent,node);
		});

		insert(I_("string"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<std::string>>(&parent,node);
		});

		insert(I_("default"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<uint32_t>>(&parent,node);
		});

	}

	Factory::Controller::~Controller() {
	}

	Factory::Controller & Factory::Controller::getInstance() {

		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;

	}

	const char * Factory::validate_name(const char *name) {

		if(!(name && *name))
			throw runtime_error("Can't use empty name");

		// Check for reserved names.
		static const char *reserved_names[] = {
			"state",
			"agent",
			"attribute"
		};

		for(size_t ix = 0; ix < (sizeof(reserved_names)/sizeof(reserved_names[0])); ix++) {
			if(!strcasecmp(name,reserved_names[ix])) {
				string message("Can't use reserved name \'");
				message += name;
				message += "\'";
				throw std::runtime_error(message);
			}
		}

		return name;

	}

	void Factory::Controller::load(std::shared_ptr<Abstract::Agent> parent, const pugi::xml_node &node) {

		// First load sub nodes
		for(auto child : node.children()) {

			//
			// Load agents.
			//
			const char *name = child.name();
			if(strcasecmp(name,"agent") == 0) {
				name = child.attribute("type").as_string();
			}

			if(!name && *name)
				continue;

			try {
				lock_guard<recursive_mutex> lock(guard);
				auto factory = agents.find(name);
				if(factory != agents.end()) {
					auto agent = factory->second.create(*parent,child);
					this->load(agent,child);
					parent->children.push_back(agent);
				}
			} catch(const std::exception &e) {

				cerr << e.what() << endl;

			}

			//
			// Load other nodes.
			//
			{
				lock_guard<recursive_mutex> lock(guard);
				auto factory = nodes.find(name);
				if(factory != nodes.end()) {
					factory->second.apply(parent,child);
				}

			}

		}

		// And then, load states.
		for(pugi::xml_node state = node.child("state"); state; state = state.next_sibling("state")) {
			parent->append_state(state);
		}


	}

	void Factory::Controller::insert(const Quark &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		lock_guard<recursive_mutex> lock(guard);
		Factory::validate_name(name.c_str());
		agents.insert(std::make_pair(name.c_str(),Agent(method)));
	}

	void Factory::Controller::insert(const Quark &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory) {
		lock_guard<recursive_mutex> lock(guard);
		Factory::validate_name(name.c_str());
		nodes.insert(std::make_pair(name.c_str(),Node(factory)));
	}

	void Factory::insert(const Quark &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		Factory::Controller::getInstance().insert(name,method);
	}

	void Factory::insert(const Quark &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory) {
		Factory::Controller::getInstance().insert(name,factory);
	}

	void Factory::load(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_document &doc) {

		auto factory = Factory::Controller::getInstance();

		for(pugi::xml_node node = doc.child("config"); node; node = node.next_sibling("config")) {
			factory.load(agent,node);
		}

	}
	*/

}
