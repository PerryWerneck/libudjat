

#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <udjat/tools/file.h>
#include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {
		Module::Controller::getInstance().load();
	}

	void Module::load(const char *name) {

		Config::Value<string> configured("modules",name,name);

#ifdef DEBUG
		cout << "Alias: '" << name << "' Module: '" << configured.c_str() << "'" << endl;
#endif // DEBUG

		Module::Controller::getInstance().load((string{STRINGIZE_VALUE_OF(PLUGIN_DIR) "/"} + configured + ".so").c_str());
	}

	void Module::Controller::load() {

		File::List(STRINGIZE_VALUE_OF(PLUGIN_DIR) "/*.so").forEach([this](const char *filename){

			try {

				cout << "module\tLoading '" << filename << "'" << endl;

				load(filename);

			} catch(const exception &e) {

				cerr << "module\t" << e.what() << endl;

			}

		});


	}

	Module * Module::Controller::load(const char *filename) {

		dlerror();

		void * handle = dlopen(filename,RTLD_NOW|RTLD_LOCAL);
		if(handle == NULL) {
			throw runtime_error(dlerror());
		}

		 Module * (*init)(void);

		 Module * module = nullptr;
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

		 return module;
	}

}

