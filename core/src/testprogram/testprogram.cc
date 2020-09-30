/**
 * @file src/core/agent/testprogram/testprogram.cc
 *
 * @brief Test agent engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/agent.h>
 #include <vector>
 #include <string>
 #include <iostream>
 #include "../main/private.h"
 #include <civetweb.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	/// @brief Root agent.
	std::shared_ptr<Abstract::Agent> root = make_shared<Abstract::Agent>();

	// Load XML
	{
		pugi::xml_document doc;
		doc.load_file("./src/main/agent/test.xml");

		auto factory = Factory::Controller::getInstance();

		for(pugi::xml_node node = doc.child("config"); node; node = node.next_sibling("config")) {
			factory.load(root,node);
		}
	}

	root->start();

	root->foreach([](Udjat::Abstract::Agent &agent) {
		cout << "Agent: " << agent.getName() << endl;
	});

	cout 	<< endl
			<< root->as_json().toStyledString()
			<< endl << endl
			<< root->getState()->as_json().toStyledString()
			<< endl << endl;


	root->stop();


	return 0;
}
