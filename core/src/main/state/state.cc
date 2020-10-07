/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/event.h>
 #include <iostream>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	const char * Abstract::State::levelNames[] = {
		"undefined",
		"unimportant",
		"ready",
		"warning",
		"error",
		"critical",

		nullptr
	};

	Abstract::State::Level Abstract::State::getLevelFromName(const char *name) {

		for(size_t ix=0; Abstract::State::levelNames[ix]; ix++) {
			if(!strcasecmp(name,Abstract::State::levelNames[ix]))
				return (Abstract::State::Level) ix;
		}

		throw runtime_error("Unknown level");

	}

	Abstract::State::State(const Level l, const char *m, const char *d) : level(l), summary(m), detailed(d) {

#ifdef DEBUG
		cout << "Creating state \"" << this->summary << "\" " << levelNames[this->level] << endl;
#endif // DEBUG

	}

	Abstract::State::State(const pugi::xml_node &node) :
		Abstract::State(getLevelFromName(getAttribute(node,"level").as_string(levelNames[unimportant])),
		getAttribute(node,"summary").as_string()) {

		this->href = Udjat::getAttribute(node,"href").as_string();

	}

	Abstract::State::~State() {

		// Delete my events.
		for(auto event : events) {
			delete event;
		}

	}

	void Abstract::State::get(Json::Value &value) const {

		value["summary"] = summary.c_str();
		value["href"] = href.c_str();
		value["level"] = levelNames[level];

	}

	Request & Abstract::State::get(Request &request) {

		request.push("summary",summary.c_str());
		request.push("detailed",detailed.c_str());
		request.push("level",levelNames[level]);

		return request;

	}

	Json::Value Abstract::State::as_json() const {

		Json::Value node;
		get(node);
		return node;

	}

	const char * Abstract::State::to_string(const Abstract::State::Level level) {

		if(level > (sizeof(Abstract::State::levelNames) / sizeof(Abstract::State::levelNames[0])))
			return "Invalid";
		return levelNames[level];
	}

	void Abstract::State::activate(const Agent &agent) noexcept {

		for(auto event:events) {
			try {

				event->set(agent, *this, true);

			} catch(const std::exception &e) {

				cerr << "Error firing event \"" << *event << "\": " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error firing event \"" << *event << "\"" << endl;

			}

		}
	}

	void Abstract::State::deactivate(const Agent &agent) noexcept {
		for(auto event:events) {

			try {

				event->set(agent, *this, false);

			} catch(const std::exception &e) {

				cerr << "Error firing event \"" << *event << "\": " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error firing event \"" << *event << "\"" << endl;

			}

		}
	}

}
