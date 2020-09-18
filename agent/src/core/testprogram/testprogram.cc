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

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	auto controller = Udjat::Abstract::Agent::load("./agent");

	controller->start();

	auto agent = controller->find({"intvalue","subkey"});

	controller->foreach([](Udjat::Abstract::Agent &agent) {
		cout << "Agent: " << agent.getName() << endl;
	});

	cout << controller->find("intvalue")->as_json().toStyledString() << endl;

	controller->stop();

	return 0;
}
