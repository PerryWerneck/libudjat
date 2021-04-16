
#pragma once

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/agent.h>

	namespace Udjat {

		/// @brief Udjat module.
		class UDJAT_API Module {
		private:
			class Controller;

			Quark name;
			void *handle;

		protected:

			struct {

				/// @brief The module name.
				const char *name;

				/// @brief The module description.
				const char *description;

				/// @brief The module version.
				const char *version;

				/// @brief The bugreport address.
				const char *bugreport;

				/// @brief The package URL.
				const char *url;

			} package;

		public:

			/// @brief Load all modules.
			static void load();

			/// @brief Load a single module.
			static Module * load(const char *filename);

			Module(const Quark &name, void *handle = nullptr);
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
		UDJAT_API Udjat::Module * udjat_module_init(void *handle);

	}
