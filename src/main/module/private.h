
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <udjat/tools/mainloop.h>
#include <list>

using namespace std;

namespace Udjat {

	class Module::Controller {
	private:
		friend class MainLoop;

		static recursive_mutex guard;

		list<Module *> modules;

	public:
		Controller();
		~Controller();

		static Controller & getInstance();

		void insert(Module *module);
		void remove(Module *module);

		void load();
		Module * load(const char *filename);

		void unload();

		void start() noexcept;
		void stop() noexcept;
		void reload() noexcept;

		void getInfo(Response &response);

	};


}
