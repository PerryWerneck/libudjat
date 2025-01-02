/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 // References:
 // https://codereview.stackexchange.com/questions/173929/modern-c-singleton-template

 #pragma once
 #include <udjat/defs.h>
 #include <list>
 #include <mutex>
 #include <functional>
 #include <udjat/tools/logger.h>
 #include <system_error>
 #include <iostream>

 namespace Udjat {

	namespace Singleton {

		/// @brief Template for container of pointers.
		/// @tparam T The class for container.
		/// @tparam P The pointer type (T *).
		/// @tparam L The standard container (std::list<P>)
		template <class T, class P = T *, class L = std::list<P>>
		class Container {
		protected:

			L objects;
			std::mutex guard;

		public:

			Container() {
			}

			Container(const Container&) = delete;
			Container& operator=(const Container &) = delete;
			Container(Container &&) = delete;
			Container & operator=(Container &&) = delete;

			virtual ~Container() {
				clear();
			}

			static Container<T> & getInstance() {
				static Container<T> instance;
				return instance;
			}

			virtual void clear() {
				while(size()) {
					P object = back();
					delete object;
					remove(object); // Just in case.
				}
			}

			P back() {
				std::lock_guard<std::mutex> lock(guard);
				return objects.back();
			}

			P front() {
				std::lock_guard<std::mutex> lock(guard);
				return objects.front();
			}

			inline size_t size() noexcept {
				std::lock_guard<std::mutex> lock(guard);
				return objects.size();
			}

			inline void push_back(P object) noexcept {
				std::lock_guard<std::mutex> lock(guard);
				objects.push_back(object);
			}

			inline void remove(P object) noexcept {
				std::lock_guard<std::mutex> lock(guard);
				objects.remove(object);
			}

			T & operator[](const char *name) const {
				std::lock_guard<std::mutex> lock(*(const_cast<std::mutex *>(&guard)));
				for(auto object : objects) {
					if(*object == name) {
						return *object;
					}
				}
				throw std::system_error(ENOENT,std::system_category(),Logger::Message("Cant find '{}'",name));
			}

			virtual P find(const char *name) const noexcept {
				std::lock_guard<std::mutex> lock(*(const_cast<std::mutex *>(&guard)));
				for(auto object : objects) {
					if(*object == name) {
						return object;
					}
				}
				return nullptr;
			}

			inline bool for_each(const std::function<bool(const T &object)> &method) const {
				std::lock_guard<std::mutex> lock(*(const_cast<std::mutex *>(&guard)));
				for(auto object : objects) {
					if(method(*object)) {
						return true;
					}
				}
				return false;
			}

			inline typename L::iterator begin() {
				return objects.begin();
			}

			inline typename L::iterator end() {
				return objects.end();
			}

			inline typename L::const_iterator begin() const {
				return objects.begin();
			}

			inline typename L::const_iterator end() const {
				return objects.end();
			}
		};

		template <class T, class P = T *, class L = std::list<P>>
		class NamedContainer : public Container<T,P,L> {
		private:
			const char *cntl_name;

		public:
			NamedContainer(const char *name) : cntl_name{name} {
			}

			std::ostream & info() const {
				return std::cout << cntl_name << "\t";
			}

			std::ostream & warning() const {
				return std::clog << cntl_name << "\t";
			}

			std::ostream & error() const {
				return std::cerr << cntl_name << "\t";
			}

			inline const char * name() const {
				return cntl_name;
			}

		};

	}

 }

