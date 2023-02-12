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

		template <class T, class L = std::list<T *>>
		class Container {
		protected:

			L objects;

			std::mutex guard;

			Container() {
			}

		public:

			Container(const Container&) = delete;
			Container& operator=(const Container &) = delete;
			Container(Container &&) = delete;
			Container & operator=(Container &&) = delete;

			virtual ~Container() {
				for(auto object : objects) {
					delete object;
				}
			}

			/*
			static Container<T> & getInstance() {
				static Container<T> instance;
				return instance;
			}
			*/

			virtual void clear() {

				while(objects.size()) {
					T *object = objects.back();
					delete object;
					{
						// Just in case.
						std::lock_guard<std::mutex> lock(guard);
						objects.remove(object);
					}
				}

			}

			inline void size() const noexcept {
				return objects.size();
			}

			inline void push_back(T *object) noexcept {
				std::lock_guard<std::mutex> lock(guard);
				objects.push_back(object);
			}

			inline void remove(T *object) noexcept {
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

			virtual const T * find(const char *name) const noexcept {
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

		};

		template <class T, class L = std::list<T *>>
		class NamedContainer : public Container<T,L> {
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

