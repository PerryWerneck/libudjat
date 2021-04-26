
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
			bool 	started;

			/// @brief The module name.
			Quark	name;

			/// @brief The module handle.
			void *handle;

		protected:

			/// @brief Module information.
			const ModuleInfo *info;

		public:

			/// @brief Load all modules.
			static void load();

			/// @brief Load a single module.
			static Module * load(const char *filename);

			/// @brief List modules.
			static void getInfo(Response &response);

			Module(const Quark &name);
			virtual ~Module();

			static Module::Controller & getController();

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
