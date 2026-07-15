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
#include <cstring>
#include <set>
#include <udjat/module.h>
#include <memory>
#include <ostream>

namespace Udjat {

	/// @brief Container for unit tests.
	class UDJAT_API UnitTests {
	public:

		/// @brief Loaded module.
		class Module : public std::string {
		private:
#ifdef _WIN32
			HMODULE handle;
#else
			void *handle;
#endif	
		public:
#ifndef _WIN32
			Module(void *h, const char *f) : std::string{f}, handle{h} {				
			}

			void *dlsym(const char *name) const noexcept;
#endif	
			~Module();

		};

		/// @brief Unit test runner.
		class Worker  {
		private:

			friend class UnitTests;

			const char *label;					///< @brief The test label (for menu).
			const char *option = nullptr;		///< @brief The test option (for command line).
			
			/// @brief The callback to run test, return message if the test was ok, exception if failed.
			std::function<const std::string (std::ostream &)> call = nullptr;

		public:
			Worker(const char *o, const char *l, const std::function<const std::string (std::ostream &)> &c) :
				label{l}, option{o}, call{c} {
			}

			Worker(const char *l, const std::function<const std::string (std::ostream &)> &c) :
				label{l}, call{c} {
			}

			bool operator<(const Worker& other) const {
				return strcasecmp(label,other.label) < 0;
			}

			bool inline operator==(const Worker& other) const {
				return strcasecmp(label,other.label) == 0;
			}

			bool operator==(const char *opt) const;
			
			inline const std::string exec(std::ostream &stream) const {
				return call(stream);
			}

			inline const char *c_str() const noexcept {
				return label;
			}

			inline const size_t size() const noexcept {
				return strlen(label);
			}

		};

		UnitTests();
		~UnitTests();

		/// @brief Load unit tests.
		void load() noexcept;

		/// @brief Interactive mode.
		void interactive(const char *title = nullptr) noexcept;

		/// @brief Run test.
		/// @param name Test name to run, nullptr to run all.
		void run(const char *name = nullptr) noexcept;

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

		void for_each(const std::function<void(const char *option, const char *label)> &func) const;

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
