/**
 * @file
 *
 * @brief Implements the event methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/atom.h>
 #include <udjat/event.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Event::Event(const Atom &n) : name(n) {
	}

	Abstract::Event::Event(const char *n) : Event(Atom(n)) {
	}

	Abstract::Event::Event(const pugi::xml_node &node) : Event(Atom(node.attribute("name"))) {
	}

	Abstract::Event::~Event() {
	}

	bool Abstract::Event::emit(const Abstract::Agent UDJAT_UNUSED(&agent), bool UDJAT_UNUSED(level_has_changed)) {
		return true;
	}

	bool Abstract::Event::emit(const Abstract::Agent UDJAT_UNUSED(&agent), const Abstract::State UDJAT_UNUSED(&state), bool UDJAT_UNUSED(active)) {
		return true;
	}

}
