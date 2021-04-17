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
 #include <udjat/tools/file.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/url.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static void test_file_agent() {
	Udjat::File::Agent file("x.txt");
	MainLoop::getInstance().run();
}

static void test_agent_parser() {

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

	File::List("${PWD}/src/main/agent/*.xml").forEach([root_agent](const char *filename){

		cout << endl << "Loading '" << filename << "'" << endl;
		pugi::xml_document doc;
		doc.load_file(filename);
		root_agent->load(doc);

	});

	Module::load();
	Udjat::start();

	run_civetweb();

	/*
	{
		Response response;
		Worker::work("agent",Request("/intvalue"),response);
		cout << response.toStyledString() << endl;
	}
	*/

	{
	}

	/*
	{
		Json::Value response(Json::objectValue);
		root_agent->get(response,true,true);
		cout << response.toStyledString() << endl;
	}
	*/

	Udjat::stop();

}

int main(int argc, char **argv) {

	// Logger::redirect();

	// test_file_agent();
	//test_agent_parser();

	// Udjat::run();

	{
		Response rsp;

		auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

		Worker::getInfo(rsp);

		cout << rsp.toStyledString() << endl;
	}

	/*
	{
		URL::insert(make_shared<URL::Protocol>("http","80"));

		Udjat::URL url;
		url.assign("http://www.google.com/udjaturl/test?xxx");
	}
	*/

	return 0;
}
