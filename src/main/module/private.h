
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <list>

using namespace std;

namespace Udjat {

	class Module::Controller {
	private:
		static recursive_mutex guard;

		list<Module *> modules;

	public:
		Controller();
		~Controller();

		static Controller & getInstance();

		void insert(Module *module);
		void remove(Module *module);

		void start() noexcept;
		void stop() noexcept;
		void reload() noexcept;

	};


}
