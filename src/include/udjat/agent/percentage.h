/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

 /// @brief Declares agent with a percentage.
 
 #include <udjat/defs.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/xml.h>
 #include <udjat/agent.h>	

 namespace Udjat {

	/// @brief Percentual value agent.
	class Percentage : public Udjat::Agent<float> {
	protected:

		struct StateDescription {
			float value;			///< @brief State max value.
			const char * name;		///< @brief State name.
			Udjat::Level level;		///< @brief State level.
			const char * summary;	///< @brief State summary.
			const char * body;		///< @brief State description
		};

		/// @brief Append a new state from description.
		/// @param state The new state description.
		/// @param current The minimum value for the new state, will be updated with the state maximum value.
		void append(const StateDescription &state, float &current);

	public:
		Percentage(const char *name, const char *label = nullptr, const char *summary = nullptr);
		Percentage(const pugi::xml_node &node, const char *label = nullptr, const char *summary = nullptr);

		virtual ~Percentage();

		std::string to_string() const noexcept override;
		std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override;

	};

 }

