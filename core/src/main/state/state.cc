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
		Abstract::State(getLevelFromName(getAttribute(node,"level").as_string(levelNames[0])),getAttribute(node,"summary").as_string()) {

		this->href = Udjat::getAttribute(node,"href").as_string();

	}

	void Abstract::State::activate(Agent &agent) {
	}

	void Abstract::State::deactivate(Agent &agent) {
	}

}
