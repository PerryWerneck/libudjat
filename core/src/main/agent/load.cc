/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &root) {

		for(pugi::xml_node node : root) {

			try {

				cout << "---> " << node.name() << endl;


			} catch(const std::exception &e) {

				cerr << "Erro loading node '" << node.name() << "': " << e.what() << endl;

			}

		}

	}

	void Abstract::Agent::load(const pugi::xml_document &doc) {

		auto agent = this;

		for(pugi::xml_node root = doc.child("config"); root; root = root.next_sibling("config")) {
			load(root);
		}

	}


}
