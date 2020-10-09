
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
			inline size_t operator() (const char *a) const {

				// https://stackoverflow.com/questions/7666509/hash-function-for-string
				size_t value = 5381;

				for(const char *ptr = a; *ptr; ptr++) {
					value = ((value << 5) + value) + *ptr;
				}

				return value;
			}
		};

		class Equal {
		public:
			inline bool operator() (const char *a, const char *b) const {
				return strcmp(a,b) == 0;
			}
		};

		/// @brief Dynamic allocated strings.
		std::unordered_set<const char *, Hash, Equal> allocated;

		/// @brief Internal (static) allocated strings (just stored).
		std::unordered_set<const char *, Hash, Equal> stored;

	public:

		Controller() {
		}

		~Controller() {

			for(auto item : allocated) {
				delete[] item;
			}

		}

		static Controller & getInstance();

		const char * find(const char *value, bool copy) {

			lock_guard<recursive_mutex> lock(guard);

			// Search on stored strings.
			{
				auto it = stored.find(value);

				if(it != stored.end()) {
#ifdef DEBUG
					cout << "Found stored " << (*it) << " (searching for " << value << "\")" << endl;
#endif // DEBUG
					return *it;
				}
			}

			// Search on allocated strings.
			{
				auto it = allocated.find(value);

				if(it != allocated.end()) {
#ifdef DEBUG
					cout << "Found " << (*it) << " (searching for " << value << "\")" << endl;
#endif // DEBUG
					return *it;
				}
			}

			if(copy) {

				// Copy string to internal storage.
				size_t sz = strlen(value);
				char *str = new char[sz+1];
				memset(str,0,sz+1);
				strncpy(str,value,sz+1);

				auto result = allocated.insert(str);
#ifdef DEBUG
				cout << "Inserting new string \"" << (*result.first) << "\"" << endl;
#endif // DEBUG

				return * result.first;

			}

			auto result = stored.insert(value);
#ifdef DEBUG
			cout << "Storing string \"" << (*result.first) << "\"" << endl;
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
		this->value = Controller::getInstance().find(str,true);
	}

	Atom::Atom(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string(),true);
	}

	Atom Atom::getFromStatic(const char *str) {
		Atom atom;
		atom.value = Controller::getInstance().find(str,false);
		return atom;
	}


	Atom::Atom(const std::string &str) : Atom(str.c_str()) {
	}

	Atom::Atom(const Atom &src) {
		this->value = src.value;
	}

	Atom::Atom(const Atom *src) {
		this->value = src->value;
	}

	Atom & Atom::operator=(const char *str) {
		this->value = Controller::getInstance().find(str,true);
		return *this;
	}

	Atom & Atom::operator=(const std::string &str) {
		this->value = Controller::getInstance().find(str.c_str(),true);
		return *this;
	}

	Atom & Atom::operator=(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string(),true);
		return *this;
	}

	const char * Atom::c_str() const {
		return value ? value : "";
	}


}
