
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

		mutable std::mutex guard;

		struct Handler {
#ifdef _WIN32
			HMODULE handle = NULLHANDLE;
#else
			void * handle = NULL;
#endif

			/// @brief Unload module on service/application stop?
			bool unload = false;

			/// @brief Delete module on service/application stop?
			bool cleanup = false;

			/// @brief Pointer to the module object.
			Module * module = nullptr;

			constexpr Handler(Module *m) noexcept : module{m} {
			}

		};

		Handler & handler(Module *module);

		std::list<Handler> handlers;

		/// @brief Find path from module name.
		/// @param name Module name.
		/// @return Module path or empty string if not found.
		static std::string locate(const char *name,const std::vector<std::string> &paths) noexcept;

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
		static void * getSymbol(HMODULE hModule, const char *name, bool required = true);
#else
		static void * getSymbol(void *handle, const char *name, bool required = true);
#endif

		Module * find_by_filename(const char *filename);
		Module * find_by_name(const char *name);

		static Controller & getInstance();

		void clear();

		void push_back(Module *module);
		void remove(Module *module);

		/// @brief Load module by xml definition.
		/// @param node Module definitions.
		/// @return true if the node was parsed and should be ignored by the caller.
		bool parse(const XML::Node &node) override;

		/// @brief Load module by filename.
		/// @param filename The module filename.
		/// @return true if the module was already loaded.
		/// @retval true The module was already loaded.
		static bool load(const std::string &filename, const XML::Node &node);
		
		bool for_each(const std::function<bool(Module &module)> &method);

	};


}
