
#pragma once

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/agent.h>

	namespace Udjat {

		/// @brief Udjat module.
		class UDJAT_API Module {
		private:
			class Controller;

			void *handle;
			Quark name;

		public:

			static Module * load(const char *filename);

			Module(const Quark &name, void *handle = nullptr);
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
		Module * UDJAT_API udjat_module_init(void *handle);

	}
