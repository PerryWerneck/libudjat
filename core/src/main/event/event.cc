/**
 * @file
 *
 * @brief Implements the event methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/tools/atom.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/xml.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Event::Event(const Atom &n) : name(n) {
		Controller::getInstance();

		try {

			Config::File &config = Config::File::getInstance();

			retry.first	= config.get("event","delay-before-start",retry.first);
			retry.interval = config.get("event","delay-before-retry",retry.interval);
			retry.limit	= config.get("event","max-retries",retry.limit);

		} catch(const std::exception &e) {

			cerr << name << "\tError '" << e.what() << "' loading defaults" << endl;

		} catch(...) {

			cerr << name << "\tUnexpected error loading defaults" << endl;

		}
	}

	Abstract::Event::Event(const char *n) : Event(Atom(n)) {
	}

	Abstract::Event::Event(const pugi::xml_node &node) : Event(Atom(node.attribute("name"))) {

		retry.first = getAttribute(node,"delay-before-start").as_uint(retry.first);
		retry.interval = getAttribute(node,"delay-before-retry").as_uint(retry.interval);
		retry.limit	= getAttribute(node,"max-retries").as_uint(retry.limit);

	}

	Abstract::Event::~Event() {
		clear();
	}

	void Abstract::Event::clear() {
		Controller::getInstance().remove(this);
	}

	void Abstract::Event::set(const Abstract::Agent &agent, bool level_has_changed) {

#ifdef DEBUG
		cout << *this << "\tEvent fired by agent " << (level_has_changed ? "level" : "value") << " change" << endl;
#endif // DEBUG

		Controller::getInstance().insert(this,&agent,nullptr,[this](const Abstract::Agent &agent, const Abstract::State &state) {
			emit(agent,state);
		});
	}

	void Abstract::Event::set(const Abstract::Agent &agent, const Abstract::State &state, bool active) {

#ifdef DEBUG
		cout << *this << "\tEvent fired by state " << (active ? "activation" : "deactivation") << endl;
#endif // DEBUG

		if(active) {
			Controller::getInstance().insert(this,&agent,&state,[this](const Abstract::Agent &agent, const Abstract::State &state) {
				emit(agent,state);
			});
		} else {
			clear();
		}

	}

	/// @brief Fire event for agent/state.
	void Abstract::Event::emit(const Abstract::Agent UDJAT_UNUSED(&agent), const Abstract::State UDJAT_UNUSED(&state)) {
		emit();
	}

	void Abstract::Event::emit() {
		cerr << PACKAGE_NAME << "\tEvent '" << *this << "' is useless" << endl;
	}

}
