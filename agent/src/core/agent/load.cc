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

	static const struct TypeLoader {
		const char *name;
		std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent *parent, const pugi::xml_node &node)> factory;
	} TypeLoaders[] = {

		{
			"integer",
			[](Abstract::Agent *parent, const pugi::xml_node &node) {
#ifdef DEBUG
				cout << "********** Name=" << node.attribute("name").as_string() << " type=int" << endl;
#endif // DEBUG
				return make_shared<Agent<int>>(parent,node);
			}
		},

		{
			"boolean",
			[](Abstract::Agent *parent, const pugi::xml_node &node) {
				return make_shared<Agent<bool>>(parent,node);
			}
		},

		{
			"string",
			[](Abstract::Agent *parent, const pugi::xml_node &node) {
				return make_shared<Agent<std::string>>(parent,node);
			}
		},

		{
			"default",
			[](Abstract::Agent *parent, const pugi::xml_node &node) {
#ifdef DEBUG
				cout << "********** Name=" << node.attribute("name").as_string() << " type=default" << endl;
#endif // DEBUG
				return make_shared<Abstract::Agent>(parent,node);
			}
		}

	};

	void Abstract::Agent::load(const pugi::xml_node &toplevel) {

		for(pugi::xml_node node = toplevel.child("agent"); node; node = node.next_sibling("agent")) {

			const char *type = node.attribute("type").as_string("default");

#ifdef DEBUG
			cout << "Name=" << node.attribute("name").as_string() << " type=" << node.attribute("type").as_string() << endl;
#endif // DEBUG

			for(size_t ix = 0; ix < (sizeof(TypeLoaders)/sizeof(TypeLoaders[0]));ix++) {

				if(!strcasecmp(type,TypeLoaders[ix].name)) {

					// TODO: Use a module-based factory method.
					std::shared_ptr<Abstract::Agent> agent = TypeLoaders[ix].factory(this,node);

					// Create agent states
					for(pugi::xml_node state = node.child("state"); state; state = state.next_sibling("state")) {
						agent->append_state(state);
					}

					// Add agent on the list
#ifdef DEBUG
					cout << "Inserting \"" << agent->name << "\"" << endl;
#endif // DEBUG
					agent->state = agent->find_state();
					children.push_back(agent);
					break;
				}

			}

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

