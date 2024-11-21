/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 // #define SERVICE_TEST 1
 // #define APPLICATION_TEST 1
 // #define OBJECT_TEST 1

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tests.h>
 #include <udjat/tools/system.h>
 #include <udjat/tools/method.h>
 #include <udjat/tools/response.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

 static const Udjat::ModuleInfo moduleinfo { "Test program" };

#if defined(SERVICE_TEST)
 int main(int argc, char **argv) {
	return Testing::Service::run_tests(argc,argv,moduleinfo);
 }

#elif defined(APPLICATION_TEST)
 int main(int argc, char **argv) {
	return Testing::Application::run_tests(argc,argv,moduleinfo);
 }

#elif defined(OBJECT_TEST)
 int main(int argc, char **argv) {

	bindtextdomain(GETTEXT_PACKAGE, STRINGIZE_VALUE_OF(LOCALEDIR));
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	debug("Locale set to ",STRINGIZE_VALUE_OF(LOCALEDIR),"/",GETTEXT_PACKAGE);
	Logger::redirect();
	Logger::verbosity(9);
	Logger::console(true);

	/*
	printf("------------------------\n");
	cout << String{}.set_byte(10000.0) << endl;
	cout << String{}.set_byte(0.0) << endl;
	cout << String{}.set_byte(229.0) << endl;
	printf("------------------------\n");
	*/

	/*
	URL url{"http://example.com?v1=10&v2=20"};

	cout << "sz=" << url[url.size()-1] << endl;
	cout << "Arg v1='" << url["v1"] << "'" << endl;
	cout << "Arg v2='" << url["v2"] << "'" << endl;
	*/

	/*
	String test{"create table if not exists urls\n(id integer primary key, inserted timestamp default CURRENT_TIMESTAMP, url text, action text, payload text)"};

	std::vector<String> lines = test.split("\n");

	for(auto &line : lines) {
		cout << "Line: '" << line << "'" << endl;
	}
	*/

	// File::Path{"/usr/share/icons/gnome/"}.find("*/computer-symbolic.svg",true);

	/*
	Request request{"/1/2/3/4"};

	for(size_t ix = 1;ix < 5; ix++) {
		String s{request[ix]};
		cout << "Arg[" << ix << "]='" << s << "'" << endl;
		cout << "pop='" << request.pop() << "'" << endl << endl;
	}
	*/

	/*
	{
		String text{"+Hello cruel world+/How are you today?"};

		cout << "Escaped:   '" << text.escape() << "'" << endl;
		cout << "Unescaped: '" << text.unescape() << "'" << endl;
	}
	*/

	/*
	{
		enum Test{ v1, v2, v3 };

		Udjat::Agent<Test> ag{"test",v1};

	}
	*/

	/*
	File::Path file{"/tmp/xpto.txt"};
	cout << file.name() << endl;
	*/

	/*
	string name = Udjat::Icon{"udjat"}.filename();
	cout << "Found: '" << name << "'" << endl;
	*/

	{
		URL url;

		url = "file:///tmp/xa/xb/";
		url += "/xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

		url = "file:///tmp/xa/xb";
		url += "xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

		url = "file:///tmp/xa/xb";
		url += "/xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

		url = "file:///tmp/xa/xb/";
		url += "xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

		url = "file:///tmp/xa/xb/";
		url += "./xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

		url = "file:///tmp/xa/xb";
		url += "./xc";
		cout << "URL= '" << url.c_str() << "'" << endl;

	}

	/*
	{
		set<String> strings;
		strings.emplace("test");
		strings.emplace("second");
		strings.emplace("Test");
		strings.emplace("aaa");

		for(auto &str : strings) {
			cout << "--> '" << str << "'" << endl;
		}

		String v1{"teste"}, v2{"Teste"};

		cout << (v1 == v2 ? "equal" : "not equal") << endl;

	}
	*/

	/*
	MainLoop::getInstance().TimerFactory(1000,[]{
		cout << "timer\tActivated!" << endl;
		return true;
	});

	MainLoop::getInstance().run();
	*/

	debug("User data dir is '",Application::UserDataDir{"test"}.c_str(),"'");

	return 0;
}
#else
 int main(int argc, char **argv) {
	return Testing::run(argc,argv,moduleinfo,[](Udjat::Application &){

		Logger::String{"----> System CPE is '",Udjat::System::cpe().c_str(),"'"}.trace();

		MainLoop::getInstance().TimerFactory(1000,[]{
			cout << "-[ On Timer ]---------------------------------------------------" << endl;

			Request request{"/"};
			Response response{Udjat::sh};			
			Method::find("agent").call(request,response);
			cout << "Response:" << endl << response << endl;

			cout << "----------------------------------------------------------------" << endl;
			return false;
		});

	});
 }
#endif
