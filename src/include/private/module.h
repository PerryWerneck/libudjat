
#pragma once

#include <config.h>
#include <udjat/module/abstract.h>
#include <udjat/tools/mainloop.h>
#include <udjat/tools/value.h>
#include <udjat/tools/container.h>
#include <udjat/tools/xml.h>
#include <mutex>

using namespace std;

namespace Udjat {

	class Module::Controller : private XML::Parser {
	private:
		friend class MainLoop;

		/// @brief The loaded modules.
		Container<Module> modules;

#ifdef _WIN32

		void close(HMODULE module);
		bool deinit(HMODULE handle);
		void unload(HMODULE handle, const string &name, const string &description) const;

#else
		bool deinit(void *handle);
		void unload(void *handle, const string &name, const string &description) const;

#endif // _WIN32

	public:
		Controller();
		~Controller();

#ifdef _WIN32
		static void * get_symbol(HMODULE hModule, const char *name, bool required = true);

		template <typename ret, typename... args>
		inline ret call(HMODULE hModule, const char *name, args... a) noexcept {
			ret (*func)(args...) = (ret (*)(args...)) get_symbol(hModule, name);
			return func(a...);
		}
 
		template <typename ret, typename... args>
		inline auto getfunc(HMODULE hModule, const char *name,bool required = true) noexcept {
			return reinterpret_cast<ret(*)(args...)>(get_symbol(hModule, name, required));
		}
#else
		static void * get_symbol(void *handle, const char *name, bool required = true);

		template <typename ret, typename... args>
		inline ret call(void *handle, const char *name, args... a) noexcept {
			ret (*func)(args...) = (ret (*)(args...)) get_symbol(handle,name,true);
			return func(a...);
		}
 
		template <typename ret, typename... args>
		inline auto getfunc(void *handle,const char *name,bool required = true) noexcept {
			return reinterpret_cast<ret(*)(args...)>(get_symbol(handle,name,required));
		}
#endif

		Module * find_by_filename(const char *filename);
		Module * find_by_name(const char *name);

		static Controller & getInstance();

		/// @brief Find path from module name.
		/// @param name Module name.
		/// @return Module path or empty string if not found.
		static std::string locate(const char *name,const std::vector<std::string> &paths = Module::search_paths()) noexcept;

		/// @brief Unload all modules.
		void unload();

		/// @brief Load module by xml definition.
		/// @param node Module definitions.
		/// @return true if the node was parsed and should be ignored by the caller.
		bool parse(const XML::Node &node) override;

		/// @brief Load module by filename.
		/// @param filename The module filename.
		/// @retval true The module was already loaded.
		/// @retval false The module was loaded.
		bool load(const std::string &filename, const XML::Node &node);
		
		bool for_each(const std::function<bool(Module &module)> &method);

		inline void push_back(Module *module) {
			modules.push_back(module);
		}

		inline void remove(Module *module) {
			modules.remove(module);
		}

	};


}
