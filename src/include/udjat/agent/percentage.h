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
 #include <udjat/tools/percentage.h>
 #include <udjat/agent.h>	
 #include <udjat/agent/state.h>
 #include <udjat/tools/value.h>
 #include <sstream>
 #include <iomanip>
 #include <memory>

 namespace Udjat {

	template <>
	class UDJAT_API Agent<Percentage> : public Abstract::Agent {
	private:

		/// @brief Agent value.
		Percentage value;

	protected:

		typedef Agent<Percentage> super;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<float>>> states;

		void for_each(const std::function<void(const Abstract::State &state)> &method) const override {
			for(auto state : states) {
				method(*state);
			}
		}

		/// @brief Insert State with predefined value.
		std::shared_ptr<Abstract::State> StateFactory(const XML::Node &node, Percentage value) {
			auto state = std::make_shared<State<float>>(node, value);
			states.push_back(state);
			return state;
		}

		/// @brief Insert State with predefined range.
		std::shared_ptr<Abstract::State> StateFactory(const XML::Node &node, Percentage from, Percentage to) {
			auto state = std::make_shared<State<float>>(node, from, to);
			states.push_back(state);
			return state;
		}

		std::shared_ptr<Abstract::State> computeState() override {
			for(auto state : states) {
				if(state->compare((float) this->value))
					return state;
			}
			return Abstract::Agent::computeState();
		}

		Udjat::Value & get(Udjat::Value &value) const override {
			return value.setFraction(this->value);
		}

		/// @brief Start with value.
		/// @param value The initial value to set.
		inline void start(const Percentage value) {
			this->value = value;
			Abstract::Agent::start();
		}

		struct StateDescription {
			float value;			///< @brief State max value.
			const char * name;		///< @brief State name.
			Udjat::Level level;		///< @brief State level.
			const char * summary;	///< @brief State summary.
			const char * body;		///< @brief State description
		};

		inline void append(const StateDescription &state, float &current) {
			this->push_back(
				std::make_shared<State<float>>(
					state.name,
					current,
					state.value,
					state.level,
					state.summary,
					state.body
				)
			);
		}

	public:

		Agent(const XML::Node &node, const Percentage v = 0) : Abstract::Agent{node}, value{from_xml<float>(node, (float) v)} {
		}

		Agent(const char *name = "") : Abstract::Agent{name}, value{0.0} {
		}

		Agent(const char *name, const Percentage v) : Abstract::Agent{name}, value{v} {
		}

		bool set(const Percentage value) {

			if(value == this->value)
				return updated(false);

			this->value = value;
			return updated(true);
		}

		bool assign(const char *value) override {
			return set(atof(value));
		}

		inline bool operator ==(const Percentage value) const noexcept {
			return this->value == value;
		}

		inline Agent & operator = (const Percentage value) {
			set(value);
			return *this;
		}

		inline Agent & operator = (const char *value) {
			assign(value);
			return *this;
		}

		Percentage get() const noexcept {
			return value;
		}

		/// @brief Insert State.
		std::shared_ptr<Abstract::State> StateFactory(const XML::Node &node) override {
			auto state = std::make_shared<State<float>>(node);
			states.push_back(state);
			return state;
		}

		std::string to_string() const noexcept override {
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << (((float) (value)) * 100.0) << "%";
			return stream.str();
		}

	};
	
 }

