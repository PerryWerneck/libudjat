

#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {

		static const char *path = STRINGIZE_VALUE_OF(PLUGIN_DIR);

		// TODO: Refactory using 'glob' and a global object.

		/*
		DIR *dir = opendir(path);
		if(!dir) {
			cerr << "modules\tCan't open '" << path << "': " << strerror(errno) << endl;
			return;
		}

		cout << "modules\tLoading from '" << path << "'" << endl;

		struct dirent **namelist;
		int qtdFiles = scandirat(dirfd(dir), ".", &namelist, 0, alphasort);

		for(int file = 0; file < qtdFiles; file++) {

			#error Memory leak!!!

			if(*namelist[file]->d_name == '.')
				continue;

			string filename(path);
			filename += '/';
			filename += namelist[file]->d_name;

			try {

#ifdef DEBUG
				cout << "Loading '" << filename << "'" << endl;
#endif // DEBUG

				load(filename.c_str());

			} catch(const exception &e) {

				cerr << "Can't load '" << filename << "': " << e.what() << endl;

			}


		}

		closedir(dir);
		*/

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

			if(!module->handle) {
				clog << "Strange module initialization on '" << filename << "'" << endl;
				module->handle = handle;
			}

		 } catch(const exception &e) {

			dlclose(handle);
			handle = NULL;
			throw;

		 }

		 return module;
	}

}

