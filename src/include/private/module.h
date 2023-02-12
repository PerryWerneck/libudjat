
#pragma once

#include <config.h>
#include <udjat/module.h>
#include <udjat/tools/mainloop.h>
#include <udjat/tools/value.h>
#include <udjat/tools/singleton.h>
#include <list>

using namespace std;

namespace Udjat {

	class Module::Controller : public Singleton::Container<Module> {
	private:
		friend class MainLoop;

		/// @brief Find path from module name.
		/// @param name Module name.
		/// @return Module path or empty string if not found.
		static std::string locate(const char *name) noexcept;

		void init(const std::string &filename, const pugi::xml_node &node);

#ifdef _WIN32

		static void * getSymbol(HMODULE hModule, const char *name, bool required = true);
		static Module * init(HMODULE hModule);
		static Module * init(HMODULE hModule, const pugi::xml_node &node);

		void close(HMODULE module);
		bool deinit(HMODULE handle);
		void unload(HMODULE handle, const string &name, const string &description) const;

#else

		static void * getSymbol(void *handle, const char *name, bool required = true);
		static Module * init(void *handle);
		static Module * init(void *handle, const pugi::xml_node &node);

		bool deinit(void *handle);
		void unload(void *handle, const string &name, const string &description) const;

#endif // _WIN32

	public:
		Controller();
		~Controller();

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void clear() override;

		/// @brief Load module by xml definition.
		/// @param node Module definitions.
		/// @return true if the module was already loaded.
		bool load(const pugi::xml_node &node);
		bool load(const std::string &filename, bool required);

	};


}
