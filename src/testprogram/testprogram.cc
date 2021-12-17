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
 #include <udjat/worker.h>
 #include <udjat/tools/disk/stat.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mimetype.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/factory.h>
 #include <udjat/url.h>
 #include <udjat/alert.h>
 #include <random>
 #include <ctime>
 #include <cstdlib>

#ifndef _WIN32
 #include <netdb.h>
#endif // _WIN32

 #include <udjat/win32/registry.h>

 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/system/info.h>

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

	/*
	Udjat::File::Text("/proc/cpuinfo").forEach([](const string &line){
		cout << line << endl;
	});
	*/

	/*
	Udjat::File::Agent file("test.xml");
	Udjat::run();
	*/

	for(auto it : Udjat::File::Text("test.xml")) {
		cout << it->c_str() << endl;
	}

	cout << "------" << endl;
}

static void test_agent_parser() {

	class Factory : public Udjat::Factory {
	public:
		Factory() : Udjat::Factory("random") {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>() {
					cout << "Creating random Agent" << endl;
					load(node);
				}

				bool refresh() override {
					set( ((unsigned int) rand()) % limit);
					return true;
				}

			};

			parent.insert(make_shared<RandomAgent>(node));

			return true;
		}

	};

	static Factory factory;

	// Load agent descriptions.
	{
		auto root_agent = Abstract::Agent::init("${PWD}/*.xml");

		cout << "http://localhost:8989/api/1.0/info/modules.xml" << endl;
		cout << "http://localhost:8989/api/1.0/info/workers.xml" << endl;
		cout << "http://localhost:8989/api/1.0/info/factory.xml" << endl;
		cout << "http://localhost:8989/api/1.0/agent.xml" << endl;
		cout << "http://localhost:8989/api/1.0/alerts.xml" << endl;

		for(auto agent : *root_agent) {
			cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << ".xml" << endl;
		}

	}

	Alert::init();

	/*
	MainLoop::getInstance().insert("none",10,[](const time_t now) {
		MainLoop::getInstance().quit();
		return false;
	});
	*/

	Udjat::SystemService(PACKAGE_NAME).start();

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

	MainLoop::getInstance().insert(0,1000,[](){
		SubProcess::start("ls -ltr");
		return false;
	});

	Udjat::MainLoop::getInstance().run();

}

int main(int argc, char **argv) {

	// Setup locale
	setlocale( LC_ALL, "" );

	{
		Udjat::Win32::Registry registry;

		cout << "Registry= '" << registry.get("test","") << "'" << endl;

		return 0;
	}

	/*
	// Redirect output to log file
	Logger::redirect();
	Module::load();
	Alert::init();

	cout << PACKAGE_NAME << "\tThe application name is '" << Application::Name() << "'" << endl;
	cout << PACKAGE_NAME << "\tThe application datadir is '" << Application::DataDir() << "'" << endl;
	cout << PACKAGE_NAME << "\tThe application libdir is '" << Application::LibDir() << "'" << endl;
	cout << PACKAGE_NAME << "\tThe application modules dir is '" << Application::LibDir("modules") << "'" << endl;
	*/

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
	test_agent_parser();
	// test_sub_process();
	// test_file_agent();
	// test_url();

	/*
	{
		System::Info info;

		cout << endl << endl << "Total ram=" << info.totalram << endl << endl;
	}
	*/

	/*
	{
		Disk::Stat stat("/dev/sda");
		cout << "SDA.blocksize = " << stat.getBlockSize() << endl;
	}
	*/

	/*
	{
		URL::insert(make_shared<URL::Protocol>("http","80"));

		Udjat::URL url;
		url.assign("http://www.google.com/udjaturl/test?xxx");
	}
	*/

	/*
	{
		TimeStamp t;

		t = "2021-01-02 03:04:05";

		cout << "-------------------" << endl << t << endl << "------------------------" << endl;

	}
	*/

	Module::unload();

	return 0;
}
