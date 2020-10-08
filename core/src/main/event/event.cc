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

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Event::Event(const Atom &n) : name(n) {
		Controller::getInstance();
	}

	Abstract::Event::Event(const char *n) : Event(Atom(n)) {
	}

	Abstract::Event::Event(const pugi::xml_node &node) : Event(Atom(node.attribute("name"))) {
	}

	Abstract::Event::~Event() {
		clear();
	}

	void Abstract::Event::clear() {
		Controller::getInstance().remove(this);
	}

	void Abstract::Event::set(const Abstract::Agent &agent, bool level_has_changed) {
		Controller::getInstance().insert(this,&agent,nullptr,[this](const Abstract::Agent &agent, const Abstract::State &state) {
			emit(agent,state);
		});
	}

	void Abstract::Event::set(const Abstract::Agent &agent, const Abstract::State &state, bool active) {

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
		cerr << "Event \"" << *this << "\" don't do anything" << endl;
	}

}
