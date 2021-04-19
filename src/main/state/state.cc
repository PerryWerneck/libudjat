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
 #include <udjat/alert.h>
 #include <iostream>
 #include <udjat/tools/timestamp.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const char * levelNames[] = {
		"undefined",
		"unimportant",
		"ready",
		"warning",
		"error",
		"critical",

		nullptr
	};

	Abstract::State::Level Abstract::State::getLevelFromName(const char *name) {

		for(size_t ix=0; levelNames[ix]; ix++) {
			if(!strcasecmp(name,levelNames[ix]))
				return (Abstract::State::Level) ix;
		}

		throw runtime_error("Unknown level");

	}

	Abstract::State::State(const Level l, const char *m, const char *b) : level(l), summary(m), body(b),activation(0) {

#ifdef DEBUG
		cout << "Creating state \"" << this->summary << "\" " << levelNames[this->level] << endl;
#endif // DEBUG

	}

	Abstract::State::State(const pugi::xml_node &node) :
		Abstract::State(
			getLevelFromName(Attribute(node,"level",false).as_string(levelNames[unimportant])),
			Attribute(node,"summary",true).as_string()) {

		this->uri = Udjat::Attribute(node,"uri").as_string();

	}

	Abstract::State::~State() {

	}

	void Abstract::State::get(Json::Value &value) const {

		value["summary"] = summary.c_str();
		value["body"] = body.c_str();
		value["uri"] = uri.c_str();

		if(this->activation)
			value["activation"] = TimeStamp(this->activation).to_string(TIMESTAMP_FORMAT_JSON);
		else
			value["activation"] = this->activation;

		// Set level information
		getLevel(value);

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::State::get(const Request &request, Response &response) const {
		this->get(response);
	}
	#pragma GCC diagnostic pop

	void Abstract::State::getLevel(Json::Value &value) const {
		Json::Value level;
		level["value"] = (uint32_t) this->level;
		level["label"] = levelNames[this->level];
		value["level"] = level;
	}

	const char * Abstract::State::to_string(const Abstract::State::Level level) {

		if(level > (sizeof(levelNames) / sizeof(levelNames[0])))
			return "Invalid";
		return levelNames[level];
	}

	void Abstract::State::activate(const Agent &agent) noexcept {

		this->activation = time(nullptr);

		for(auto alert : alerts) {
			try {

				Alert::set(alert, agent, *this, true);

			} catch(const std::exception &e) {

				cerr << agent << "\tError '" << e.what() << "' activating alert '" << alert->c_str() << "'" << endl;

			} catch(...) {

				cerr << agent << "\tUnexpected error activating alert '" << alert->c_str() << "'" << endl;

			}

		}

	}

	void Abstract::State::deactivate(const Agent &agent) noexcept {

		for(auto alert : alerts) {

			try {

				Alert::set(alert, agent, *this, false);

			} catch(const std::exception &e) {

				cerr << agent << "\tError '" << e.what() << "' deactivating alert '" << alert->c_str() << "'" << endl;

			} catch(...) {

				cerr << agent << "\tUnexpected error activating alert '" << alert->c_str() << "'" << endl;

			}

		}
	}

}
