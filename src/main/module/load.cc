
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

		Config::Value<vector<string>> modules("modules","load","");

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

		string path{Object::getAttribute(node,"modules","path","")};
		bool required = Object::getAttribute(node,"modules","required",true);

		if(!path.empty()) {
			Module::Controller::getInstance().load(path.c_str(),required);
			return;
		}

		const char * name = Object::getAttribute(node,"name","");
		if(!*name) {
			throw runtime_error("Required attribute 'name' is missing");
		}

		load(name,required);

	}

	void Module::load(const char *name, bool required) {

		Config::Value<string> configured("modules",name,(string{"udjat-module-"} + name).c_str());

#ifdef _WIN32
		// Scan WIN32 module paths.
		Application::LibDir libdir;

		// FIXME: Detect the right path.
		string paths[] = {
			Config::Value<string>("modules","primary-path",Application::LibDir("modules").c_str()),
#if defined(__x86_64__)
			// 64 bit detected
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw64\\lib\\udjat-modules\\" PACKAGE_VERSION "\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw64\\lib\\udjat-modules\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw64/lib/udjat-modules/" PACKAGE_VERSION "/"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw64/lib/udjat-modules/"
			),
#elif  defined(__i386__)
			// 32 bit detected
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw32\\lib\\udjat-modules\\" PACKAGE_VERSION "\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw32\\lib\\udjat-modules\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw32/lib/udjat-modules/" PACKAGE_VERSION "/"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw32/lib/udjat-modules/"
			),
#else
			Config::Value<string>(
				"modules",
				"secondary-path",
				(libdir + "udjat-modules\\" + PACKAGE_VERSION "\\").c_str()
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				(libdir + "udjat-modules\\").c_str()
			),
#endif
		};
#else
		// Scan Linux module paths.
		string paths[] = {
			Config::Value<string>("modules","primary-path",Application::LibDir("modules/" PACKAGE_VERSION).c_str()),
			Config::Value<string>("modules","secondary-path",Application::LibDir("modules").c_str()),
#ifdef LIBDIR
			Config::Value<string>("modules","common-path",STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "-modules/" PACKAGE_VERSION "/").c_str(),
#endif //LIBDIR
		};
#endif // _WIN32

		for(size_t ix = 0; ix < (sizeof(paths)/sizeof(paths[0]));ix++) {
			string filename = paths[ix] + configured + MODULE_EXT;
			if(access(filename.c_str(),R_OK) == 0) {
				auto module = Module::Controller::getInstance().load(filename.c_str(),required);
				if(module) {
					return;
				}
			}
#ifdef DEBUG
			else {
				cout << "modules\tNot found in " << filename << " (" << ix << ")" << endl;
			}
#endif // DEBUG

		}

		cerr << "modules\tCant find module '" << name << "'" << endl;
		return;
	}

	Module * Module::Controller::load(const char *filename, bool required) {

		// Check if is already loaded.
		for(auto module : modules) {

			if(!strcasecmp(module->filename().c_str(),filename)) {
#ifdef DEBUG
				cout << "module\tModule '" << module->name << "' is already loaded" << endl;
#endif // DEBUG
				return module;
			}

		}

		// First check if the file is acessible.
		if(access(filename,R_OK) != 0) {

			// Check if the module exists.

			string message{"Cant access '"};
			message += filename;
			message += "' - ";
			message += strerror(errno);
			if(required) {
				throw runtime_error(message);
			}

			clog << "module\t" << message << endl;
			return nullptr;
		}

		// Load module
		cout << "module\tLoading '" << filename << "'" << endl;

		Module * (*init)(void);
		Module * module = nullptr;

#ifdef _WIN32

		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
		HMODULE handle = LoadLibrary(filename);
		if(!handle) {
			string message = Win32::Exception::format(GetLastError());

			if(required) {
				throw runtime_error(string{"Cant load '"} + filename + "' - " + message.c_str());
			}

			clog << "module\tCant load '" << filename << "': " << message << endl;

			return nullptr;
		}

		try {

			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wcast-function-type"
			init = (Module * (*)(void)) GetProcAddress(handle,"udjat_module_init");
			#pragma GCC diagnostic pop
			if(!init) {
				throw Win32::Exception("Cant get module init method");
			}

			module = init();
			if(!module) {
				throw runtime_error("Can't initialize module");
			}

			module->handle = handle;

		} catch(...) {

			FreeLibrary(handle);
			throw;

		}

#else

		dlerror();

		void * handle = dlopen(filename,RTLD_NOW|RTLD_LOCAL);
		if(handle == NULL) {
			if(required) {
				throw runtime_error(dlerror());
			}
			clog << "module\t" << dlerror() << endl;
			return nullptr;
		}

		try {

			init = (Module * (*)(void)) dlsym(handle,"udjat_module_init");
			auto err = dlerror();
			if(err)
				throw runtime_error(err);

			module = init();
			if(!module) {
				throw runtime_error("Can't initialize module");
			}

			module->handle = handle;

		} catch(...) {

			dlclose(handle);
			handle = NULL;
			throw;

		}

#endif // _WIN32

		return module;

	}

}

