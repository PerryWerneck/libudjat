
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <udjat/tools/file.h>
#include <udjat/tools/quark.h>
#include <list>

using namespace std;

namespace Udjat {

	class File::Controller : public Module {
	private:

		/// @brief Mutex to prevent multiple access to file list.
		static recursive_mutex guard;

		/// @brief Inotify instance.
		int instance;

		struct Watch {

			/// @brief Inotify file handle
			int wd;

			/// @brief The full filename.
			Quark path;

		};

	public:
		Controller();
		~Controller();

		static Controller & getInstance();

		void insert(const Quark &path, File *file);
		void remove(const Quark &path, File *file);

		void reload() override;

	};


}
