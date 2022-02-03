
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
	#include <unistd.h>
	#define MODULE_EXT ".so"
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {
		Module::Controller::getInstance().load();
	}

	void Module::load(const char *name, bool required) {

		Config::Value<string> configured("modules",name,(string{"udjat-module-"} + name).c_str());

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
		clog << "module\tLoading '" << filename << "'" << endl;

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

		} catch(...) {

			dlclose(handle);
			handle = NULL;
			throw;

		}

#endif // _WIN32

		return module;

	}

}

