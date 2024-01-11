/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/value.h>
 #include <udjat/module/info.h>
 #include <udjat/agent.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/file.h>
 #include <vector>
 #include <cstdarg>

 namespace Udjat {

	/// @brief Udjat module.
	class UDJAT_API Module {
	private:

		/// @brief The module name.
		const char *name;

		/// @brief When true the module is never unloaded.
		bool keep_loaded = false;

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
		const ModuleInfo &_info;

	protected:

		typedef Udjat::Module super;

		Module(const char *name, const ModuleInfo &info);

		Module(const ModuleInfo &info) : Module(info.name,info) {
		}

		Module(const char *name, const ModuleInfo *info) : Module(name,*info) {
		}

		/// @brief Navigate on module options.
		static void options(const XML::Node &node, std::function<void(const char *name, const char *value)> call);

	public:

		bool operator==(const char *name) const noexcept {
			return strcasecmp(this->name,name) == 0;
		}

		inline const char * description() const noexcept {
			return _info.description;
		}

		inline int build() const noexcept {
			return _info.build;
		}

		inline const char * gettext_package() const noexcept {
			return _info.gettext_package;
		}

		/// @brief Preload modules from configuration file.
		/// @return true if success.
		static bool preload() noexcept;

		/// @brief Call method on every modules.
		static bool for_each(const std::function<bool(const Module &module)> &method);

		/// @brief Get module by name.
		/// @param name Module name without path or extension (ex: "udjat-module-civetweb") or alias (ex: "http").
		/// @return Pointer to module or nullptr if not found.
		static const Module * find(const char *name) noexcept;

		/// @brief Get module search paths.
		static std::vector<std::string> search_paths() noexcept;

		/// @brief Load module by path.
		/// @param name path to module filename or directory.
		/// @param required true if the module is required.
		static void load(const File::Path &path, bool required = true);

		/// @brief Load module by name.
		static bool load(const char *name, bool required = true);

		/// @brief Load module by XML node.
		static void load(const XML::Node &node);

		/// @brief Unload modules.
		static void unload();

		/// @brief Set XML document
		/// Called when a XML document is loaded.
		virtual void set(const pugi::xml_document &document);

		virtual ~Module();

		/// @brief Get module filename.
		std::string filename() const;

		/// @brief Get property value.
		/// @param key The property name.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		virtual bool getProperty(const char *key, std::string &value) const;

		virtual Value & getProperties(Value &properties) const;

		/// @brief Get module property.
		/// @param property_name The property name.
		/// @return The property value.
		std::string operator[](const char *property_name) const noexcept;

		/// @brief Execute command.
		static void exec(const char *module_name, Udjat::Value &response, const char *name, ...) __attribute__ ((sentinel));

		/// @brief Execute command.
		void exec(Udjat::Value &response, const char *name,...) const __attribute__ ((sentinel));

		/// @brief Execute command.
		virtual void exec(Udjat::Value &response, const char *name, va_list args) const;

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		/// @brief Set new root agent.
		virtual void set(std::shared_ptr<Abstract::Agent> agent) noexcept;

		/// @brief Build agent from XML node, used on tag <agent type='module-name'>
		virtual std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Build alert from XML node, used on tag <alert type='module-name'>
		virtual std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const XML::Node &node) const;

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
	UDJAT_API Udjat::Module * udjat_module_init_from_xml(const Udjat::XML::Node &node);

	/// @brief Deinitialize the module.
	/// @return true if the module can be unloaded.
	UDJAT_API bool udjat_module_deinit();

 }
