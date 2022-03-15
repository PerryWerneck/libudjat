
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
		HMODULE open(const char *name, bool required);
		void close(HMODULE module);
		Module * init(HMODULE hModule);
		bool deinit(HMODULE handle);
		void unload(HMODULE handle, const string &name, const string &description) const;
#else
		void * open(const char *name, bool required);
		void close(void *module);
		Module * init(void *handle);
		bool deinit(void *handle);
		void unload(void *handle, const string &name, const string &description) const;
#endif // _WIN32


	public:
		Controller();
		~Controller();

		static Controller & getInstance();

		void insert(Module *module);
		void remove(Module *module);

		void unload();

		void load(const char *name, bool required = true);

		const Module * find(const char *name) const noexcept;

		void getInfo(Response &response) noexcept;

	};


}
