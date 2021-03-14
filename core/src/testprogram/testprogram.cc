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

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Global system state"));

	const char * xml_filename = "./src/main/agent/test.xml";

	{
		pugi::xml_document doc;
		doc.load_file(xml_filename);
		root_agent->load(doc);
	}

	root_agent->start();

//	run_civetweb();


	{
		Response response;
		Worker::work("agent",Request(""),response);


		cout << response.toStyledString() << endl;

	}

	/*
	{
		Json::Value response(Json::objectValue);
		root_agent->get(response);
		cout << response.toStyledString() << endl;
	}
	*/

	root_agent->stop();

	return 0;
}
