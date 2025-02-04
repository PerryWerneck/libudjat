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

 #include <config.h>
 #include <udjat/defs.h>
 #include <iostream>
 #include <udjat/tools/application.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/url.h>
 #include <memory>
 #include <list>

 namespace Udjat {

 	class UDJAT_PRIVATE Updater  {
	private:
		time_t next = 0;			///< @brief Seconds for next update.
		bool update;				///< @brief true if an update was requested.
		Application::Name name;		///< @brief Application name.

		struct Settings {
			std::string filename;	///< @brief The file name.
			URL url;				///< @brief The URL for file update.
			time_t ifsuccess;		///< @brief Time to refresh the file when updated
			time_t iffailed;		///< @brief Time to refresh the file if the update fails.
			bool cache;

			Settings(const std::string &f, const XML::Node &node) :
				filename{f},
				url{node,"src",false},
				ifsuccess{node.attribute("update-timer").as_uint(0)},
				iffailed{node.attribute("update-when-failed").as_uint(ifsuccess)},
				cache{node.attribute("cache").as_bool(true)} {
			}

		};

		std::list<Settings> files;

	public:
		Updater(const char *pathname, bool force);

		/// @brief Refresh XML files (if necessary);
		/// @return true to reconfigure.
		bool refresh();

		/// @brief Verify and insert file on list.
		void push_back(const std::string &filename);

		inline size_t size() const noexcept {
			return files.size();
		}

		inline auto begin() const noexcept {
			return files.begin();
		}

		inline auto end() const noexcept {
			return files.end();
		}

		/// @brief Sort by basename and load configuration files.
		/// @param agent New root agent.
		/// @return True on success.
		bool load(std::shared_ptr<Abstract::Agent> root) noexcept;

		/// @brief Get seconds for next update.
		inline time_t wait() const noexcept {
			return next;
		}

		/// @brief Write to the 'information' stream.
		inline std::ostream & info() const {
			return std::cout << name << "\t";
		}

		/// @brief Write to the 'warning' stream.
		inline std::ostream & warning() const {
			return std::clog << name << "\t";
		}

		/// @brief Write to the 'error' stream.
		inline std::ostream & error() const {
			return std::cerr << name << "\t";
		}


 	};

 }
