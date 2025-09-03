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
 #include <udjat/tools/abstract/object.h>
 #include <memory>
 #include <vector>

 namespace Udjat {

	class UDJAT_API Activatable : public Abstract::Object {
	private:
		const char *object_name;

	protected:

		typedef Activatable super;

		constexpr Activatable(const char *name = "") : object_name{name} {
		}
	
		Activatable(const XML::Node &node);
		virtual ~Activatable();

		/// @brief Convenience method to get payload from xml
		static const char * payload(const XML::Node &node);

		/// @brief Convenience method to capture and translate exceptions.
		int exec(Udjat::Value &response, bool except, const std::function<int()> &func);

	public:

		const char * name() const noexcept override;

		inline const char *c_str() const noexcept {
			return object_name;
		}

		/// @brief Activate/deactivate by parameter.
		/// @param value true to activate, false to deactivate.
		/// @return true if the state was changed.
		bool active(bool value) noexcept;

		/// @brief Activate object.
		/// @return true if the object was activated, false if already active.
		virtual bool activate() noexcept = 0;

		/// @brief Check if object can be activated.
		/// @return true if object can be activated.
		virtual bool available() const noexcept;

		/// @brief Activate object with properties.
		/// @param object Object with properties.
		/// @return true if the object was activated, false if already active.
		virtual bool activate(const Udjat::Abstract::Object &object) noexcept;

		/// @brief Deactivate object.
		/// @return true if the object was deactivated, false if already inactive.
		virtual bool deactivate() noexcept;

		/// @brief Template for activatable containers.
		template <typename T>
		class Container {
		protected:
			struct Entry{
				T type;
				std::shared_ptr<Activatable> activatable;
			};
			std::vector<Entry> entries;

		public:
			Container() = default;
			Container(const Container&) = delete;
			Container& operator=(const Container &) = delete;
			Container(Container &&) = delete;
			Container & operator=(Container &&) = delete;

			bool push_back(const T type, std::shared_ptr<Abstract::Object> child) {
				auto obj = std::dynamic_pointer_cast<T>(child);
				if(obj) {
					entries.push_back(Entry{type,obj});
					return true;
				}
				return false;
			}

			/// @brief Navigate from activatables until 'func' returns true.
			bool for_each(const std::function<bool(const T type, std::shared_ptr<Activatable> activatable)> &func) const {
				for(auto &entry : entries) {
					if(func(entry.type,entry.activatable)) {
						return true;
					}
				}
				return false;
			}

			bool available(const T type) const noexcept {
				for(auto &entry : entries) {
					if(entry.type == type && !entry.activatable->available()) {
						return false;
					}
				}
				return true;
			}

			size_t activate(const T type, const Udjat::Abstract::Object &object) const {
				size_t count = 0;
				for(auto &entry : entries) {
					if(entry.type == type && entry.activatable->activate(object)) {
						count++;
					}
				}
				return count;
			}

			size_t activate(const T type) const {
				size_t count = 0;
				for(auto &entry : entries) {
					if(entry.type == type && entry.activatable->activate()) {
						count++;
					}
				}
				return count;
			}
			
		};
	};

 }
