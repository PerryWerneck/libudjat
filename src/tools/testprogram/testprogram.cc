/**
 * @file
 *
 * @brief Test tools.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/defs.h>
 #include <iostream>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 
 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	static const ModuleInfo moduleinfo;

	class DummyProtocol : public Udjat::Protocol {
	public:
		DummyProtocol() : Udjat::Protocol("dummy",&moduleinfo) {
		}

		/*
		std::string call(const URL &url, const HTTP::Method method, const char *payload) const override {
			cout << ">>>>>> " << endl;
			return "";
		}
		*/

	};

	DummyProtocol dummy;

	/*
	cout 	<< "Processor by value: " << Dmi::Value(Dmi::PROCESSOR_INFO,0x10).as_string() << endl
			<< "Processor by name:  " << Dmi::Value("Processor",0x10).as_string() << endl;
	*/

	/*
	{
		Atom v1 = Atom::getFromStatic("teste");
		Atom v2("teste");

		cout << "V1=" << ((void *) v1.c_str()) << " V2=" << ((void *) v2.c_str()) << endl;
	}
	*/

	/*
	{
		Logger::redirect();

		Logger logger{"sample"};

		logger.info("The first value is '{}' and the second value is '{}'",
						"First value",
						2
					);


	}
	*/

	// auto url = URL{"http://localhost/sample/path?query=1"};
	auto url = URL{"dummy+http://localhost/sample?args=1"};
	auto components = url.ComponentsFactory();

	cout << "URL:\t\t'" << url << "'" << endl;
	cout << "Scheme:\t\t'" << components.scheme << "'" << endl;
	cout << "Hostname:\t'" << components.hostname << "'" << endl;
	cout << "Service:\t'" << components.srvcname << "'" << endl;
	cout << "Port:\t\t'" << components.portnumber() << "'" << endl;
	cout << "Path:\t\t'" << components.path << "'" << endl;
	cout << "Query:\t\t'" << components.query << "'" << endl;

	cout << "Response:" << endl << url.get() << endl;

	return 0;
}
