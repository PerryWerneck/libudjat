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
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	auto controller = Udjat::Abstract::Agent::load();

	controller->start();

	auto agent = controller->find({"intvalue","subkey"});

	controller->foreach([](Udjat::Abstract::Agent &agent) {
		cout << "Agent: " << agent.getName() << endl;
	});

	cout << controller->find("intvalue")->as_json().toStyledString() << endl;


	controller->refresh();
	sleep(5);

	return 0;
}
