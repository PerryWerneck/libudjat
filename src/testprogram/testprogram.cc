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
 #include <random>
 #include <ctime>
 #include <cstdlib>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static void test_file_agent() {
	Udjat::File::Agent file("x.txt");
	MainLoop::getInstance().run();
}

static void test_agent_parser() {

	class Factory : public Abstract::Agent::Factory {
	public:
		Factory() : Abstract::Agent::Factory(Quark::getFromStatic("random")) {
			cout << "Random agent factory was created" << endl;
			srand(time(NULL));
		}

		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			public:
				RandomAgent() : Agent<unsigned int>() {
					cout << "Creating random Agent" << endl;
				}

				void refresh() override {
					set((unsigned int) rand());
					cout << "Value changes to " << get() << endl;
				}

			};

			setup(parent, node, make_shared<RandomAgent>());

		}

	};

	static Factory factory;

	auto root_agent = Abstract::Agent::set_root(make_shared<Abstract::Agent>("root","System","Application"));

	File::List("${PWD}/*.xml").forEach([root_agent](const char *filename){

		cout << endl << "Loading '" << filename << "'" << endl;
		pugi::xml_document doc;
		doc.load_file(filename);
		root_agent->load(doc);

	});

	Udjat::run();

}

int main(int argc, char **argv) {

	Module::load();

	{
		static auto module = new Module(Quark::getFromStatic("sample"));
	}

	test_agent_parser();

	/*
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
	*/

	/*
	{
		URL::insert(make_shared<URL::Protocol>("http","80"));

		Udjat::URL url;
		url.assign("http://www.google.com/udjaturl/test?xxx");
	}
	*/

	return 0;
}
