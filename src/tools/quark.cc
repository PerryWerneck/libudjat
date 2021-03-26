
#include <config.h>
#include <iostream>
#include <udjat/tools/quark.h>
#include <mutex>
#include <unordered_set>

#ifdef DEBUG
	#undef DEBUG // Disable debug messages
#endif // DEBUG

using namespace std;

namespace Udjat {

	class Quark::Controller {
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

	recursive_mutex Quark::Controller::guard;

	Quark::Controller & Quark::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	Quark::Quark(const char *str) {
		this->value = Controller::getInstance().find(str,true);
	}

	Quark::Quark(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string(),true);
	}

	Quark Quark::getFromStatic(const char *str) {
		Quark q;
		q.value = Controller::getInstance().find(str,false);
		return q;
	}


	Quark::Quark(const std::string &str) : Quark(str.c_str()) {
	}

	Quark::Quark(const Quark &src) {
		this->value = src.value;
	}

	Quark::Quark(const Quark *src) {
		this->value = src->value;
	}

	Quark & Quark::operator=(const char *str) {
		this->value = Controller::getInstance().find(str,true);
		return *this;
	}

	void Quark::set(const char *str) {
		this->value = Controller::getInstance().find(str,true);
	}

	Quark & Quark::operator=(const std::string &str) {
		this->value = Controller::getInstance().find(str.c_str(),true);
		return *this;
	}

	Quark & Quark::operator=(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string(),true);
		return *this;
	}

	const char * Quark::c_str() const {
		return value ? value : "";
	}

	size_t Quark::hash() const {

		// https://stackoverflow.com/questions/7666509/hash-function-for-string
		size_t value = 5381;

		for(const char *ptr = c_str(); *ptr; ptr++) {
			value = ((value << 5) + value) + tolower(*ptr);
		}

		return value;
	}

	void Quark::set(const char *str, const std::function<const char * (const char *key)> translate) {

		string text(str);

		auto from = text.find("${");
		while(from != string::npos) {
			auto to = text.find("}",from+3);
			if(to == string::npos) {
				throw runtime_error("Invalid ${} usage");
			}
			string key(text.c_str()+from+2,(to-from)-2);
			text.replace(from,(to-from)+1,translate(key.c_str()));
			from = text.find("${",from+2);
		}

		set(text.c_str());
	}

#ifdef HAVE_PUGIXML
	bool Quark::set(const pugi::xml_node &node, const char *xml_attribute, bool upsearch, const std::function<const char * (const char *key)> translate) {

		if(!node)
			return false;

		auto attribute = node.attribute(xml_attribute);

		if(attribute) {
			set(attribute.as_string(),translate);
			return true;
		}

		// Check children for <attribute name=>
		for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

			if(strcasecmp(xml_attribute,child.attribute("name").as_string()) == 0) {
				set(child.attribute("value").as_string(),translate);
				return true;
			}

		}

		// If upsearch is true repeat the query on parent node.
		if(upsearch) {
			return set(node.parent(),xml_attribute,true,translate);
		}

		return false;

	}

	bool Quark::set(const pugi::xml_node &node, const char *xml_attribute, bool upsearch) {

		if(!node)
			return false;

		auto attribute = node.attribute(xml_attribute);

		if(attribute) {
			set(attribute.as_string());
			return true;
		}

		// Check children for <attribute name=>
		for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

			if(strcasecmp(xml_attribute,child.attribute("name").as_string()) == 0) {
				set(child.attribute("value").as_string());
				return true;
			}

		}

		// If upsearch is true repeat the query on parent node.
		if(upsearch) {
			return set(node.parent(),xml_attribute,true);
		}

		return false;

	}
#endif // HAVE_PUGIXML

}
