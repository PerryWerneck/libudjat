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

#include <config.h>
#include <udjat/defs.h>
#include <iostream>
#include <udjat/tools/quark.h>
#include <udjat/tools/xml.h>
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

			if(!(value && *value)) {
				return nullptr;
			}

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
		static Controller instance;
		return instance;
	}

	Quark::Quark(const char *str) {
		if(str && *str) {
			this->value = Controller::getInstance().find(str,true);
		} else {
			this->value = nullptr;
		}
	}

	Quark::Quark(const pugi::xml_attribute &attribute) {
		this->value = Controller::getInstance().find(attribute.as_string(),true);
	}

	Quark::Quark(const XML::Node &node,const char *name,const char *def,bool upsearch) {
		this->value = Controller::getInstance().find(Udjat::Attribute(node,name,upsearch).as_string(def),true);
	}

	void Quark::init() {
		Controller::getInstance();
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
		if(str && *str) {
			this->value = Controller::getInstance().find(str,true);
		} else {
			this->value = nullptr;
		}
		return *this;
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

	const Quark & Quark::set(const char *str) {
		if(str && *str) {
			this->value = Controller::getInstance().find(str,true);
		} else {
			this->value = nullptr;
		}
		return *this;
	}

	const Quark & Quark::set(const char *str, const std::function<const char * (const char *key)> translate) {

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
		return *this;
	}

#ifdef HAVE_PUGIXML
	const Quark & Quark::set(const XML::Node &node, const char *xml_attribute, bool upsearch, const std::function<const char * (const char *key)> translate) {

		if(!node)
			return *this;

		auto attribute = node.attribute(xml_attribute);

		if(attribute) {
			set(attribute.as_string(),translate);
			return *this;
		}

		// Check children for <attribute name=>
		for(XML::Node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

			if(strcasecmp(xml_attribute,child.attribute("name").as_string()) == 0) {
				set(child.attribute("value").as_string(),translate);
				return *this;
			}

		}

		// If upsearch is true repeat the query on parent node.
		if(upsearch) {
			return set(node.parent(),xml_attribute,true,translate);
		}

		return *this;

	}

	const Quark & Quark::set(const XML::Node &node, const char *xml_attribute, bool upsearch) {

		if(!node)
			return *this;

		auto attribute = node.attribute(xml_attribute);

		if(attribute) {
			set(attribute.as_string());
			return *this;
		}

		// Check children for <attribute name=>
		for(XML::Node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

			if(strcasecmp(xml_attribute,child.attribute("name").as_string()) == 0) {
				set(child.attribute("value").as_string());
				return *this;
			}

		}

		// If upsearch is true repeat the query on parent node.
		if(upsearch) {
			return set(node.parent(),xml_attribute,true);
		}

		return *this;

	}
#endif // HAVE_PUGIXML

}
