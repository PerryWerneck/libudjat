
#define GNU_SOURCE
#include <config.h>
#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <udjat/tools/file.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>
#include <udjat/tools/object.h>

#ifdef _WIN32
	#define MODULE_EXT ".dll"
	#include <udjat/win32/exception.h>
#else
	#include <dlfcn.h>
	#include <unistd.h>
	#define MODULE_EXT ".so"
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {

		Config::Value<vector<string>> modules("modules","load-at-startup","");

		if(modules.empty()) {
			cout << "modules\tNo preload modules" << endl;
			return;
		}

		cout << "modules\tPreloading " << modules.size() << " module(s) from configuration file" << endl;

		Config::Value<bool> required("modules","required",false);
		for(string &module : modules) {
			load(module.c_str(),required);
		}

	}

	void Module::load(const pugi::xml_node &node) {
		Controller::getInstance().load(node);
	}

	void Module::load(const char *name, bool required) {
		Controller::getInstance().load(name,required);
	}

	void Module::Controller::load(const pugi::xml_node &node) {

		const char * name = Object::getAttribute(node,"name","");
		if(!*name) {
			throw runtime_error("Required attribute 'name' is missing");
		}

		auto handle = open(name,Object::getAttribute(node,"modules","required",true));

		if(!handle)
			return;

		try {

			auto module = init(handle,node);
			module->handle = handle;

		} catch(...) {

			close(handle);
			throw;

		}

	}

	void Module::Controller::load(const char *name, bool required) {

		auto handle = open(name,required);

		if(!handle)
			return;

		try {

			auto module = init(handle);
			module->handle = handle;

		} catch(...) {

			close(handle);
			throw;

		}

	}

}

