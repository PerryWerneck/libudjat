
#pragma once

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/agent.h>
	#include <udjat/request.h>

	namespace Udjat {

		/// @brief Udjat module.
		class UDJAT_API Module {
		private:

			/// @brief The module controller.
			class Controller;
			friend class Controller;

			/// @brief True if the module was started.
			bool started;

			/// @brief The module name.
			Quark name;

			/// @brief The module handle.
			void *handle;

		protected:

			/// @brief Information about the module.
			const ModuleInfo *info;

		public:

			/// @brief Get module controller.
			static Controller & controller();

			/// @brief Load modules.
			static void load();

			/// @brief Load module by name.
			/// @param name Module name without path or extension (ex: "udjat-module-civetweb").
			static void load(const char *name);

			/// @brief Unload modules.
			static void unload();

			/// @brief List modules.
			static void getInfo(Response &response);

			Module(const Quark &name);
			virtual ~Module();

			/// @brief Start module.
			virtual void start();

			/// @brief Reload module.
			virtual void reload();

			/// @brief Stop module.
			virtual void stop();

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
