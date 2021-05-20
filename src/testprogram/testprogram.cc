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
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/factory.h>
 #include <udjat/url.h>
 #include <udjat/alert.h>
 #include <random>
 #include <ctime>
 #include <cstdlib>
 #include <netdb.h>
 #include <udjat/tools/subprocess.h>

 using namespace std;
 using namespace Udjat;

 #pragma GCC diagnostic ignored "-Wunused-parameter"
 #pragma GCC diagnostic ignored "-Wunused-function"

//---[ Implement ]------------------------------------------------------------------------------------------

static void test_file_agent() {
	Udjat::File::Agent file("test.xml");
	MainLoop::getInstance().run();
}

static void test_file_load() {
	cout << "------" << endl;
	Udjat::File::Local("/proc/cpuinfo").forEach([](const string &line){
		cout << line << endl;
	});
	cout << "------" << endl;
}

static void test_agent_parser() {

	class Factory : public Udjat::Factory {
	public:
		Factory() : Udjat::Factory("random") {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>() {
					cout << "Creating random Agent" << endl;
					load(node);
				}

				void refresh() override {
					set((unsigned int) rand());
				}

			};

			parent.insert(make_shared<RandomAgent>(node));

		}

	};

	static Factory factory;

	// Load agent descriptions.
	{
		auto root_agent = Abstract::Agent::init("${PWD}/*.xml");

		cout << "http://localhost:8989/api/1.0/info/modules" << endl;
		cout << "http://localhost:8989/api/1.0/info/workers" << endl;
		cout << "http://localhost:8989/api/1.0/info/factory" << endl;
		cout << "http://localhost:8989/api/1.0/agent" << endl;
		cout << "http://localhost:8989/api/1.0/alerts" << endl;

		for(auto agent : *root_agent) {
			cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << endl;
		}

	}

	Alert::init();

	/*
	MainLoop::getInstance().insert("none",10,[](const time_t now) {
		MainLoop::getInstance().quit();
		return false;
	});
	*/

	Udjat::run();

	// Force agent cleanup
	Abstract::Agent::deinit();

}

static void test_url() {
	// Udjat::URL url("http://localhost");
	Udjat::URL url("file://test.xml");
	auto response = url.get();
	cout << "Response was: " << response->getStatusCode() << " " << response->getStatusMessage() << endl;
	cout << response->c_str() << endl;
}

static void test_sub_process() {

	MainLoop::getInstance().insert(0,1,[](const time_t now){
		SubProcess::start("ls -ltr");
		return false;
	});

	Udjat::run();

}

int main(int argc, char **argv) {

	// Setup locale
	setlocale( LC_ALL, "" );

	// Redirect output to log file
	Logger::redirect();
	Module::load();
	Alert::init();

	/*
	{
		static auto module = new Module("sample");
	}
	*/

	/*
	{
		string text{"v1=${v1} v2=${v2} v3=${v3}"};

		Udjat::expand(
				text,
				[](const char *key){

					if(strcasecmp(key,"v1") == 0) {
						return string{"Value of V1"};
					} else if(strcasecmp(key,"v2") == 0) {
						return string{"Value of V2"};
					}
					return string{"${}"};
				}
		);

		cout 	<< "Expand= '"
				<< text
				<< "'" << endl;

	}
	*/


	// test_file_load();
	// test_agent_parser();
	// test_sub_process();
	// test_file_agent();
	test_url();

	/*

	//test_agent_parser();

	// Udjat::run();

	{
		Response rsp;

		auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

		Worker::getInfo(rsp);

		cout << rsp.toStyledString() << endl;
	}
	*/

	/*
	{
		URL::insert(make_shared<URL::Protocol>("http","80"));

		Udjat::URL url;
		url.assign("http://www.google.com/udjaturl/test?xxx");
	}
	*/

	Module::unload();

	return 0;
}
