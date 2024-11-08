
#pragma once

#include <config.h>
#include <udjat/module/abstract.h>
#include <udjat/tools/mainloop.h>
#include <udjat/tools/value.h>
#include <udjat/tools/singleton.h>
#include <list>
#include <vector>

using namespace std;

namespace Udjat {

	class Module::Controller : public Singleton::Container<Module> {
	private:
		friend class MainLoop;

		/// @brief Find path from module name.
		/// @param name Module name.
		/// @return Module path or empty string if not found.
		static std::string locate(const char *name,const std::vector<std::string> &paths) noexcept;

		void init(const std::string &filename, const XML::Node &node);

		Module * find_by_filename(const char *filename);

		Module * find_by_name(const char *name);

#ifdef _WIN32

		static void * getSymbol(HMODULE hModule, const char *name, bool required = true);

		void close(HMODULE module);
		bool deinit(HMODULE handle);
		void unload(HMODULE handle, const string &name, const string &description) const;

#else

		static void * getSymbol(void *handle, const char *name, bool required = true);

		bool deinit(void *handle);
		void unload(void *handle, const string &name, const string &description) const;

#endif // _WIN32

	public:
		Controller();
		~Controller();

#ifdef _WIN32
		static Module * init(HMODULE hModule);
		static Module * init(HMODULE hModule, const XML::Node &node);
#else
		static Module * init(void *handle);
		static Module * init(void *handle, const XML::Node &node);
#endif

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void clear() override;

		/// @brief Load module by xml definition.
		/// @param node Module definitions.
		/// @return true if the module was already loaded.
		bool load(const XML::Node &node);

		/// @brief Load module by filename.
		/// @param filename The module filename.
		/// @return true if the module was already loaded.
		/// @retval true The module was already loaded.
		bool load(const std::string &filename, bool required);

		/// @brief Load module by name.
		bool load(const char *name, bool required);

	};


}
