

#include "private.h"
#include <sys/types.h>
#include <dirent.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::load() {

		static const char *path = STRINGIZE_VALUE_OF(PLUGIN_DIR);

		DIR *dir = opendir(path);
		if(!dir) {
			cerr << "modules\tCan't open '" << path << "': " << strerror(errno) << endl;
			return;
		}

		cout << "modules\tLoading from '" << path << "'" << endl;

		struct dirent **namelist;
		int qtdFiles = scandirat(dirfd(dir), ".", &namelist, 0, alphasort);

		for(int file = 0; file < qtdFiles; file++) {

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

	}

}

