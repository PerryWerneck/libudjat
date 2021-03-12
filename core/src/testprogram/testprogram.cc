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
 #include <udjat/worker.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	Logger::redirect();

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>());

	const char * xml_filename = "./src/main/agent/test.xml";

	{
		pugi::xml_document doc;
		doc.load_file(xml_filename);
		Factory::load(root_agent,doc);
	}

	/*
	root_agent->start();
	run_civetweb();
	root_agent->stop();
	*/

	Request request("");
	Response response;

	Worker::work("agent",request,response);
	cout << response.toStyledString() << endl;

	return 0;
}
