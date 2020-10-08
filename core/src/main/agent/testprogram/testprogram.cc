/**
 * @file src/core/agent/testprogram/testprogram.cc
 *
 * @brief Test agent engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <vector>
 #include <string>
 #include <iostream>
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	auto root_agent = make_shared<Abstract::Agent>();

	set_root_agent(root_agent);

	const char * xml_filename = "./test.xml";

	{
		pugi::xml_document doc;
		doc.load_file(xml_filename);
		Factory::load(root_agent,doc);
	}

	root_agent->start();

	sleep(5);

	root_agent->stop();

	return 0;
}
