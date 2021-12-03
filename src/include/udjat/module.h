
#pragma once

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/agent.h>
	#include <udjat/request.h>

	namespace Udjat {

		/// @brief Udjat module.
		class UDJAT_API Module {
		private:

			/// @brief The module name.
			const char *name;

			/// @brief The module controller.
			class Controller;
			friend class Controller;

			/// @brief The module handle.
#ifdef _WIN32
			HMODULE handle;
#else
			void *handle;
#endif // _WIN32

		protected:

			/// @brief Information about the module.
			const ModuleInfo *info;

		public:

			/// @brief Load modules.
			static void load();

			/// @brief Load module by name or alias
			/// @param name Module name without path or extension (ex: "udjat-module-civetweb") or alias (ex: "http").
			static void load(const char *name);

			/// @brief Unload modules.
			static void unload();

			/// @brief List modules.
			static void getInfo(Response &response);

			Module(const char *name, const ModuleInfo *info);

			Module(const Quark &name, const ModuleInfo *info) : Module(name.c_str(),info) {
			}

			virtual ~Module();

		};

	}

	extern "C" {

		/// @brief Initialize module.
		/// @return Module controller.
		UDJAT_API Udjat::Module * udjat_module_init();

		/// @brief Deinitialize the module.
		/// @return true if the module can be unloaded.
		UDJAT_API bool udjat_module_deinit();

	}
