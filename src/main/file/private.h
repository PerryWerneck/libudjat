
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <udjat/tools/file.h>
#include <udjat/tools/quark.h>
#include <unistd.h>
#include <list>
#include <sys/inotify.h>

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

			/// @brief The files was modified?
			int modified;

			/// @brief Files
			list<File *> files;

			/// @brief Watch has event.
			void onEvent(uint32_t mask) noexcept;

		};

		/// @brief Active watches
		list<Watch> watches;

		Controller();

		void onEvent(struct inotify_event *event) noexcept;

	public:
		~Controller();

		static Controller & getInstance();

		void insert(File *file);
		void remove(File *file);

	};


}
