/**
 * @file src/core/agent/controller.cc
 *
 * @brief Implements the root agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <functional>
 #include <cstring>
 #include <dirent.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &node) {

		// Load children.
		Agent::Factory::getInstance().load(*this,node);

		cout << "Loading states for " << *this << endl;
		// Load states.
		for(pugi::xml_node state = node.child("state"); state; state = state.next_sibling("state")) {
			append_state(state);
		}

	}

	static int xmlFilter(const struct dirent *entry) {
		const char *ptr = strrchr(entry->d_name,'.');
		if(ptr)
			return strcasecmp(ptr+1,"xml") == 0 ? 1 : 0;
		return 0;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::load(const char *path) {

		lock_guard<std::recursive_mutex> lock(guard);

		auto controller = make_shared<Abstract::Agent::Controller>();

		if(!path)
			path = ".";

		// Load files
		struct dirent **namelist;
		int files = scandir(path, &namelist, xmlFilter, alphasort);

		for(int file = 0; file < files; file++) {

			string filename(path);
			filename += '/';
			filename += namelist[file]->d_name;

#ifdef DEBUG
			cout << "Loading " << filename << endl;
#endif // DEBUG

			try {

				pugi::xml_document doc;
				doc.load_file(filename.c_str());

				for(pugi::xml_node node = doc.child("config"); node; node = node.next_sibling("config")) {
					controller->load(node);
				}

			} catch(const exception &e) {

				cerr << "Error loading \"" << filename << "\": " << e.what() << endl;

			} catch(...) {

				cerr << "Error loading \"" << filename << "\": unexpected error" << endl;
			}

			free(namelist[file]);
		}
		free(namelist);

		return controller;

	}

}

