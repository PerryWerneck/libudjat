/**
 * @file src/core/agent/testprogram/testprogram.cc
 *
 * @brief Test agent engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <vector>
 #include <string>
 #include <udjat/request.h>
 #include <udjat/tools/logger.h>
 #include <json/value.h>
 #include <unordered_map>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	Logger::redirect();

	auto root_agent = make_shared<Abstract::Agent>();

	set_root_agent(root_agent);

	const char * xml_filename = "./src/main/agent/test.xml";

	{
		pugi::xml_document doc;
		doc.load_file(xml_filename);
		Factory::load(root_agent,doc);
	}

	root_agent->start();
	run_civetweb();
	root_agent->stop();

	return 0;
}
