
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

		/// @brief List modules.
		void getInfo(Response &response) {

			Json::Value report(Json::arrayValue);

			for(auto method : methods) {

				Json::Value value(Json::objectValue);

				value["id"] = method.second->name.c_str();
				value["name"] = method.second->info->name;
				value["description"] = method.second->info->description;
				value["version"] = method.second->info->version;
				value["bugreport"] = method.second->info->bugreport;
				value["url"] = method.second->info->url;

				report.append(value);
			}

			response["agenttypes"] = report;

		}

		void insert(const Factory *factory) {
			lock_guard<recursive_mutex> lock(guard);
#ifdef DEBUG
			cout << "Insering factory '" << factory->c_str() << "'" << endl;
#endif // DEBUG

			methods.insert(make_pair(factory->c_str(),factory));

		}

		void remove(const Factory *factory) {
			lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
			cout << "Removing factory '" << factory->c_str() << "'" << endl;
#endif // DEBUG

			auto entry = methods.find(factory->c_str());
			if(entry == methods.end())
				return;

			if(entry->second != factory)
				return;

			methods.erase(entry);

		}

		bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const {

			auto entry = methods.find(name);

			if(entry == methods.end()) {
#ifdef DEBUG
				cout << "Cant find factory for element '" << name << "'" << endl;
#endif // DEBUG
				return false;
			}

			entry->second->parse(parent,node);

			return true;
		}


	};

	recursive_mutex Abstract::Agent::Factory::Controller::guard;

	Abstract::Agent::Factory::Factory(const Quark &n) : name(n) {

		static const ModuleInfo info = {

			"",	// The module name.
			"", // The module description.
			"", // The module version.
			"", // The bugreport address.
			"", // The package URL.

		};

		this->info = &info;

		Controller::getInstance().insert(this);
	}

	Abstract::Agent::Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	void Abstract::Agent::Factory::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	bool Abstract::Agent::Factory::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

	void Abstract::Agent::Factory::setup(Abstract::Agent &parent, const pugi::xml_node &node, std::shared_ptr<Abstract::Agent> agent) const {
		agent->load(node);
		parent.insert(agent);
	}

}
