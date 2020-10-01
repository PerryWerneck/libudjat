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

	Abstract::State::State(const Level l, const char *m) : level(l), summary(m) {

#ifdef DEBUG
		cout << "Creating state \"" << this->summary << "\" " << levelNames[this->level] << endl;
#endif // DEBUG

	}

	Abstract::State::State(const pugi::xml_node &node) :
		Abstract::State(getLevelFromName(getAttribute(node,"level").as_string(levelNames[unimportant])),getAttribute(node,"summary").as_string()) {

		this->href = Udjat::getAttribute(node,"href").as_string();

	}

	void Abstract::State::get(Json::Value &value) const {

		value["summary"] = summary.c_str();
		value["href"] = href.c_str();
		value["level"] = levelNames[level];

	}

	Request & Abstract::State::get(Request &request) {

		request.push("summary",summary.c_str());
		request.push("level",levelNames[level]);

		return request;

	}

	Json::Value Abstract::State::as_json() const {

		Json::Value node;
		get(node);
		return node;

	}


	void Abstract::State::activate(const Agent &agent) {
	}

	void Abstract::State::deactivate(const Agent &agent) {
	}

}
