
#include <config.h>
#include <iostream>
#include <udjat/tools/atom.h>
#include <mutex>
#include <unordered_set>

#ifdef DEBUG
	#undef DEBUG // Disable debug messages
#endif // DEBUG

using namespace std;

namespace Udjat {

	class Atom::Controller {
	private:
		static recursive_mutex guard;

		class Hash {
		public:
			inline size_t operator() (const string *str) const {
				return std::hash<std::string>{}(*str);
			}
		};

		class Equal {
		public:
			inline bool operator() (const string *a, const string *b) const {
				return a->compare(*b) == 0;
			}
		};

		std::unordered_set<std::string *, Hash, Equal> contents;

	public:

		Controller() {

		}

		~Controller() {

			for(auto item : contents) {
				delete item;
			}

		}

		static Controller & getInstance();

		template <typename T>
		std::string * find(T value) {

			lock_guard<recursive_mutex> lock(guard);

			std::string *str = new std::string(value);

			auto it = contents.find(str);

			if(it != contents.end()) {
#ifdef DEBUG
				cout << "Found " << (*it)->c_str() << " (searching for " << str->c_str() << endl;
#endif // DEBUG
				delete str;
				return *it;
			}

			auto result = contents.insert(str);

#ifdef DEBUG
			cout << "Inserting " << (*result.first)->c_str() << endl;
#endif // DEBUG

			return *result.first;

		}


	};

	recursive_mutex Atom::Controller::guard;

	Atom::Controller & Atom::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Atom::Controller controller;
		return controller;
	}

	Atom::Atom(const char *str) {
		this->value = Controller::getInstance().find(str);
	}

	Atom::Atom(const std::string &str) {
		this->value = Controller::getInstance().find(str);
	}

	Atom::Atom(const Atom &src) {
		this->value = src.value;
	}

	Atom::Atom(const Atom *src) {
		this->value = src->value;
	}

	Atom & Atom::operator=(const char *str) {
		this->value = Controller::getInstance().find(str);
		return *this;
	}

	Atom & Atom::operator=(const std::string &str) {
		this->value = Controller::getInstance().find(str);
		return *this;
	}

	Atom & Atom::operator=(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string());
		return *this;
	}


}
