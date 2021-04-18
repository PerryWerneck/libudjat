

#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <udjat/tools/file.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {

		File::List(STRINGIZE_VALUE_OF(PLUGIN_DIR) "/*.so").forEach([](const char *filename){

			try {

#ifdef DEBUG
				cout << "Loading '" << filename << "'" << endl;
#endif // DEBUG

				load(filename);

			} catch(const exception &e) {

				cerr << "Can't load '" << filename << "': " << e.what() << endl;

			}

		});


	}

	Module * Module::load(const char *filename) {

		dlerror();

		void * handle = dlopen(filename,RTLD_NOW|RTLD_LOCAL);
		if(handle == NULL) {
			throw runtime_error(dlerror());
		}

		 Module * (*init)(void *);

		 Module * module = nullptr;
		 try {

			init = (Module * (*)(void *)) dlsym(handle,"udjat_module_init");
			auto err = dlerror();
			if(err)
				throw runtime_error(err);

			module = init(handle);
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

