
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

			/// @brief Information about the module.
			const ModuleInfo &info;

		protected:

			Module(const char *name, const ModuleInfo &info);

			Module(const char *name, const ModuleInfo *info) : Module(name,*info) {
			}

			Module(const Quark &name, const ModuleInfo &info) : Module(name.c_str(),info) {
			}

			Module(const Quark &name, const ModuleInfo *info) : Module(name.c_str(),*info) {
			}

			/// @brief Navigate on module options.
			static void options(const pugi::xml_node &node, std::function<void(const char *name, const char *value)> call);

		public:

			/// @brief Load modules from configuration file.
			static void load();

			/// @brief Call method on every modules.
			static void for_each(std::function<void(Module &module)> method);

			/// @brief Get module by name.
			/// @param name Module name without path or extension (ex: "udjat-module-civetweb") or alias (ex: "http").
			/// @return Pointer to module or nullptr if not found.
			static const Module * find(const char *name) noexcept;

			/// @brief Load module by name or alias
			/// @param name Module name without path or extension (ex: "udjat-module-civetweb") or alias (ex: "http").
			/// @param required true if the module is required.
			static void load(const char *name, bool required);

			/// @brief Load module by XML node.
			static void load(const pugi::xml_node &node);

			/// @brief Unload modules.
			static void unload();

			/// @brief List modules.
			static void getInfo(Response &response);

			/// @brief Set XML document
			/// Called when a XML document is loaded.
			virtual void set(const pugi::xml_document &document);

			virtual ~Module();

			/// @brief Get module filename.
			std::string filename() const;

		};

	}

	extern "C" {

		/// @brief Module information data.
		extern UDJAT_API const Udjat::ModuleInfo udjat_module_info;

		/// @brief Initialize module.
		/// @return Module controller.
		UDJAT_API Udjat::Module * udjat_module_init();

		/// @brief Initialize module from XML node.
		/// @return Module controller.
		UDJAT_API Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node);

		/// @brief Deinitialize the module.
		/// @return true if the module can be unloaded.
		UDJAT_API bool udjat_module_deinit();

	}
