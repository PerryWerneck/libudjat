/**
 * @file src/core/agent/testprogram/testprogram.cc
 *
 * @brief Test agent engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat.h>
 #include <udjat/agent.h>
 #include <vector>
 #include <string>
 #include <udjat/request.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <json/value.h>
 #include <udjat/worker.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	Logger::redirect();

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

	const char * xml_filename = "./src/main/agent/test.xml";

	{
		pugi::xml_document doc;
		doc.load_file(xml_filename);
		root_agent->load(doc);
	}

//	Module::load();
	Udjat::start();

//	run_civetweb();

	{
		Response response;
		//Worker::work("agent",Request(""),response);
		Worker::work("agent",Request("/intvalue"),response);


		cout << response.toStyledString() << endl;

	}

	/*
	{
		Json::Value response(Json::objectValue);
		root_agent->get(response,true,true);
		cout << response.toStyledString() << endl;
	}
	*/

	Udjat::stop();

	return 0;
}
