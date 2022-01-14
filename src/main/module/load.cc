
#define GNU_SOURCE
#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <udjat/tools/file.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>

#ifdef _WIN32
	#define MODULE_EXT ".dll"
	#include <udjat/win32/exception.h>
#else
	#include <dlfcn.h>
	#define MODULE_EXT ".so"
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {
		Module::Controller::getInstance().load();
	}

	void Module::load(const char *name, bool required) {

		Config::Value<string> configured("modules",name,name);

#ifdef DEBUG
		cout << "Alias: '" << name << "' Module: '" << configured.c_str() << "'" << endl;
#endif // DEBUG

		string filename = Application::LibDir("modules") + configured + MODULE_EXT;

#ifdef DEBUG
		cout << "Module filename: '" << filename << "'" << endl;
#endif // DEBUG

		Module::Controller::getInstance().load((Application::LibDir("modules") + configured + MODULE_EXT).c_str(),required);
	}

	void Module::Controller::load() {

		Application::LibDir libdir("modules");

#ifdef _WIN32
		mkdir(libdir.c_str());
#endif // _WIN32

		libdir += "*" MODULE_EXT;

		try {

			File::List(libdir.c_str()).forEach([this](const char *filename){

				try {

					cout << "module\tLoading '" << filename << "'" << endl;

					load(filename,true);

				} catch(const exception &e) {

					cerr << "module\t" << e.what() << endl;

				}

			});

		} catch(const std::exception &e) {

			cerr << "module\tCan't load " << libdir << ": " << e.what() << endl;
		}


	}

	Module * Module::Controller::load(const char *filename, bool required) {

		// Check if is already loaded.
		for(auto module : modules) {

			if(!strcasecmp(module->filename().c_str(),filename)) {
				cout << "module\tModule '" << module->name << "' is already loaded" << endl;
				return module;
			}

		}

		clog << "module\tLoading '" << filename << "'" << endl;

		Module * (*init)(void);
		Module * module = nullptr;

#ifdef _WIN32

		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
		HMODULE handle = LoadLibrary(filename);
		if(!handle) {
			if(required) {
				throw Win32::Exception("Cant load module");
			}
			clog << "module\tCant load '" << filename << "': " << Win32::Exception::format() << endl;
			return nullptr;
		}

		try {

			init = (Module * (*)(void)) GetProcAddress(handle,"udjat_module_init");
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

		} catch(const exception &e) {

			dlclose(handle);
			handle = NULL;
			throw;

		}

#endif // _WIN32

		return module;

	}

}

