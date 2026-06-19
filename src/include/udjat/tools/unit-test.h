/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

#include <config.h>
#include <udjat/defs.h>
#include <functional>
#include <vector>
#include <udjat/tools/container.h>
#include <string>
#include <set>
#include <udjat/module/abstract.h>
#include <memory>

namespace Udjat {

	/// @brief Container for unit tests.
	class UDJAT_API UnitTests {
	public:

		/// @brief Loaded module.
		class Module : public std::string {
		private:
#ifndef _WIN32
			void *handle;
#endif	
		public:
#ifndef _WIN32
			Module(void *h, const char *f) : std::string{f}, handle{h} {				
			}
#endif	
			~Module();

			/// @brief Get symbol from module.
			/// @details This method is used to get a symbol from the module.
			/// @param symbol The symbol name to get.
			/// @return The symbol address or nullptr if not found.
			void * get_symbol(const char *symbol_name);

			template <typename ret, typename... args>
			inline auto getfunc(const char *name) {
				return reinterpret_cast<ret(*)(args...)>(get_symbol(name));
			}

		};

		/// @brief Unit test runner.
		class Worker  {
		private:

			friend class UnitTests;

			const char *label;					///< @brief The test label (for menu).
			const char *option = nullptr;		///< @brief The test option (for command line).
			
			/// @brief The callback to run test, return true if the test was ok.
			const std::function<bool(void)> call = nullptr;

		public:
			Worker(const char *o, const char *l, const std::function<bool(void)> &c) :
				label{l}, option{o}, call{c} {
			}

			Worker(const char *l, const std::function<bool(void)> &c) :
				label{l}, call{c} {
			}

			inline bool exec() const {
				return call();
			}

			inline const char *c_str() const noexcept {
				return label;
			}

		};

		UnitTests();
		~UnitTests();

		/// @brief Load unit tests.
		void load() noexcept;

		/// @brief Interactive mode.
		void interactive() noexcept;

		/// @brief Run all loaded tests.
		void run_all() noexcept;

		template<typename... Targs>
		inline void append(const Worker &worker, Targs... Fargs) {
			append(worker);
			append(Fargs...);
		}

		inline void append(const Worker &worker) {
			workers.push_back(worker);
		}

		inline auto begin() {
			return workers.begin();
		}

		inline auto end() {
			return workers.end();
		}

		inline size_t size() const noexcept {
			return workers.size();
		}

#ifndef _WIN32
		inline void append_module(void *handle,const char *filename) {
			auto module = std::make_shared<Module>(handle,filename);
			modules.emplace(module);
		}
#endif

	private:

		struct ModuleCompare {
			bool operator()(const std::shared_ptr<Module> lhs, const std::shared_ptr<Module> rhs) const {
				// Custom logic comparing internal string data
				return strcasecmp(lhs->c_str(),rhs->c_str()); 
			}
		};

		/// @brief The loaded modules.
		std::set<std::shared_ptr<Module>,ModuleCompare> modules;

		/// @brief The optional argument groups.
		std::vector<Worker> workers;

	};

}

extern "C" {

	UDJAT_API void enum_udjat_unit_tests(Udjat::UnitTests &tests) noexcept;

}
