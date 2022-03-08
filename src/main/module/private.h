
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

#ifdef _WIN32
		HMODULE open(const char *filename, bool required);
		Module * init(HMODULE hModule);
#else
		void * open(const char *filename, bool required);
		Module * init(void *handle);
#endif // _WIN32

	public:
		Controller();
		~Controller();

		static Controller & getInstance();

		void insert(Module *module);
		void remove(Module *module);

		void unload();

		Module * load(const char *filename, bool required = true);

		const Module * find(const char *name) const noexcept;

		void getInfo(Response &response) noexcept;

	};


}
