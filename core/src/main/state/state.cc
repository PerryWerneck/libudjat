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
 #include <udjat/notification.h>
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
		Abstract::State(getLevelFromName(getAttribute(node,"level").as_string(levelNames[unimportant])),
		getAttribute(node,"summary").as_string()) {

		this->uri = Udjat::getAttribute(node,"uri").as_string();

	}

	Abstract::State::~State() {

		// Delete my events.
		for(auto event : events) {
			delete event;
		}

	}

	void Abstract::State::get(Json::Value &value) const {

		value["summary"] = summary.c_str();
		value["body"] = body.c_str();
		value["uri"] = uri.c_str();

		if(this->activation)
			value["activation"] = TimeStamp(this->activation).to_string(TIMESTAMP_FORMAT_JSON);
		else
			value["activation"] = 0;

		// Set level information
		getLevel(value);

	}

	void Abstract::State::getLevel(Json::Value &value) const {
		Json::Value level;
		level["value"] = (uint32_t) this->level;
		level["label"] = levelNames[this->level];
		value["level"] = level;
	}

	void Abstract::State::getValue(Json::Value &value) const {
		value["value"] = false;
	}

	Request & Abstract::State::get(Request &request) {

		request.push("summary",summary.c_str());
		request.push("body",body.c_str());
		request.push("level",levelNames[level]);

		return request;

	}

	Json::Value Abstract::State::as_json() const {

		Json::Value node;
		get(node);
		return node;

	}

	const char * Abstract::State::to_string(const Abstract::State::Level level) {

		if(level > (sizeof(levelNames) / sizeof(levelNames[0])))
			return "Invalid";
		return levelNames[level];
	}

	void Abstract::State::activate(const Agent &agent) noexcept {

		this->activation = time(nullptr);

		for(auto event:events) {
			try {

				event->set(agent, *this, true);

			} catch(const std::exception &e) {

				cerr << "Error firing event \"" << event << "\": " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error firing event \"" << event << "\"" << endl;

			}

		}

		if(notify && Notification::hasListeners()) {

			class Notification : public Udjat::Notification {
			public:
				Notification(const Abstract::Agent &agent, const Abstract::State &state) {

					// label = agent.getLabel();
					level = state.getLevel();
					summary = state.getSummary();
					message = state.getBody();
					uri = state.getUri();
					if(!uri)
						uri = agent.getUri();

				}
			};

			try {

				Notification(agent, *this).emit();

			} catch(const std::exception &e) {

				cerr << agent << "\tError '" << e.what() << "' emitting notification" << endl;

			} catch(...) {

				cerr << agent << "\tUnexpected error emitting notification" << endl;

			}

		}

	}

	void Abstract::State::deactivate(const Agent &agent) noexcept {
		for(auto event:events) {

			try {

				event->set(agent, *this, false);

			} catch(const std::exception &e) {

				cerr << "Error firing event \"" << event << "\": " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error firing event \"" << event << "\"" << endl;

			}

		}
	}

}
