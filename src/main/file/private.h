
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <udjat/tools/file.h>
#include <udjat/tools/quark.h>
#include <unistd.h>
#include <list>

using namespace std;

namespace Udjat {

	class File::Controller {
	private:

		/// @brief Mutex to prevent multiple access to file list.
		static recursive_mutex guard;

		/// @brief Inotify instance.
		int instance;

		struct Watch {

			/// @brief File path
			Quark name;

			/// @brief Inotify watch descriptor.
			int wd;

			/// @brief Files
			list<File *> files;

		};

		/// @brief Active watches
		list<Watch> watches;

		Controller();

	public:
		~Controller();

		static Controller & getInstance();

		void insert(File *file);
		void remove(File *file);

	};


}
