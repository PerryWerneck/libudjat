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
 #include <private/pugixml.h>
 #include <memory>
 #include <vector>

 namespace Udjat {

	/// @brief Udjat module.
	class UDJAT_API Module {
	private:

		/// @brief The module name.
		const char *module_name;

#ifdef _WIN32
		HMODULE handle = NULL;
#else
		void * handle = NULL;
#endif

		bool keep_loaded = true;
		bool keep_active = true;

	protected:

		typedef Udjat::Module super;

		/// @brief The module controller.
		class Controller;
		friend class Controller;

		struct Info {
 
			/// @brief Module build.
#ifdef BUILD_DATE
			const int build = BUILD_DATE;
#else
			const int build = 0;
#endif // BUILD_DATE

			/// @brief The module package.
#ifdef PACKAGE_NAME
			const char *name = PACKAGE_NAME;
#else
			const char *name = "";
#endif // PACKAGE_NAME

			/// @brief The module description.
#ifdef PACKAGE_DESCRIPTION
			const char *description = PACKAGE_DESCRIPTION;
#else
			const char *description = "";
#endif // PACKAGE_DESCRIPTION

		/// @brief The module version.
#ifdef PACKAGE_VERSION
			const char *version = PACKAGE_VERSION;
#else
			const char *version = "";
#endif // PACKAGE_VERSION

		/// @brief The bugreport address.
#ifdef PACKAGE_BUGREPORT
			const char *bugreport = PACKAGE_BUGREPORT;
#else
			const char *bugreport = "";
#endif // PACKAGE_BUGREPORT

		/// @brief The package URL.
#ifdef PACKAGE_URL
			const char *url = PACKAGE_URL;
#else
			const char *url = "";
#endif // PACKAGE_URL

#ifdef GETTEXT_PACKAGE
			const char * gettext_package = GETTEXT_PACKAGE;
#else
			const char * gettext_package = nullptr;
#endif // GETTEXT_PACKAGE

		} info;

#if defined(PACKAGE_NAME) && defined(PACKAGE_DESCRIPTION)
		Module(const char *name = PACKAGE_NAME, const char *description = PACKAGE_DESCRIPTION);
#elif defined(PACKAGE_NAME)
		Module(const char *name = PACKAGE_NAME, const char *description = "");
#else
		Module(const char *name, const char *description);
#endif

	public:

		/// @brief Initialize module engine.
		static void initialize() noexcept;

		/// @brief Build module from filename.
		/// @param filename Path to the .so ou .dll file with module.
		static bool load(const char *filename, const Properties &props = Properties{});

		/// @brief Enable auto cleanup of the module at exit.
		inline void autoclean(bool enabled = true) noexcept {
			keep_loaded = !enabled;
			keep_active = !enabled;
		}

		/// @brief Find path from module name.
		/// @param name Module name.
		/// @return Module path or empty string if not found.
		static std::string locate(const char *name) noexcept;

		bool operator==(const char *name) const noexcept {
			return strcasecmp(this->module_name,name) == 0;
		}

		inline const char * name() const noexcept {
			return module_name;
		}
		
		inline const char * description() const noexcept {
			return info.description;
		}

		inline int build() const noexcept {
			return info.build;
		}

		inline const char * gettext_package() const noexcept {
			return info.gettext_package;
		}

		/// @brief Call method on every modules.
		/// @param method The method to call on every module.
		/// @return true if the method has returned true for any module.
		/// @note This method is used to call a method on every module.
		/// @note The method should return true if the scan should be stopped.
		/// @note The method should return false if the scan should continue.
		static bool for_each(const std::function<bool(Module &module)> &method);

		/// @brief Get module by name.
		/// @param name Module name without path or extension (ex: "udjat-module-civetweb") or alias (ex: "http").
		/// @return Pointer to module or nullptr if not found.
		static const Module * find(const char *name) noexcept;

		/// @brief Get module search paths.
		static std::vector<std::string> search_paths() noexcept;

		/// @brief Load module by path.
		/// @param name path to module filename or directory.
		/// @param required true if the module is required.
		static bool load(const std::string &name, const Properties &props = Properties{});

		static bool load(const Properties &props = Properties{});
		
		/// @brief Unload modules.
		static void unload();

		virtual ~Module();

		/// @brief Write Module paths on trace file.
		/// @param url_prefix Prefix for URL (built by server module);
		virtual void trace_paths(const char *url_prefix) const noexcept;

		/// @brief Get filename for pointer
		static std::string filename(const void *ptr, const char *def = nullptr);

		/// @brief Get module filename.
		std::string filename() const;

		/// @brief Get property value.
		/// @param key The property name.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		virtual bool getProperty(const char *key, std::string &value) const;

		virtual Value & get_properties(Value &properties) const;

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

		/// @brief Set root agent, called every time the root agent changes.
		/// @param agent The new root agent.
		virtual void set(std::shared_ptr<Abstract::Agent> agent);

		/// @brief Get symbol from module.
		/// @details This method is used to get a symbol from the module.
		/// @param symbol The symbol name to get.
		/// @return The symbol address or nullptr if not found.
		void * get_symbol(const char *symbol_name, bool required = true);

		template <typename ret, typename... args>
		inline auto getfunc(const char *name, bool required = true) {
			return reinterpret_cast<ret(*)(args...)>(get_symbol(name,required));
		}
 		
	};

 }

 extern "C" {

	/// @brief Run unit test.
	/// @details This function is used to run unit tests from the command line.
	/// @note This function is used by the test program and should not be used in production code.
	/// @param name The test name to run. If null, all tests are run.
	/// @return 0 if success, -1 on error.
	[[deprecated("Use UnitTest")]] UDJAT_API int run_unit_test(const char *name);

	/// @brief Run unit test.
	/// @details This function is used to run unit tests from the command line.
	/// @note This function is used by the test program and should not be used in production code.
	/// @param name The test name to run. If null, all tests are run.
	/// @return 0 if success, -1 on error.
	[[deprecated("Use UnitTest")]] UDJAT_API int run_udjat_unit_test(const char *name);

	/// @brief Initialize module from properties.
	/// @return Module controller.
	UDJAT_API Udjat::Module * udjat_module_init(const Udjat::Properties &props);

	/// @brief Deinitialize the module.
	/// @return true if the module can be unloaded.
	UDJAT_API bool udjat_module_deinit();

	/// @brief Get symbol from application.
	/// @details This method is used to get a symbol from the module.
	/// @param symbol The symbol name to get.
	/// @return The symbol address or nullptr if not found.
	void * symbol(const char *symbol, bool required = true);

 }

