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

 #include <udjat/defs.h>
 #include <string>
 #include <cstdlib>

 namespace Udjat {

	 inline int to_value(const char *str, int &value) {
		return (value = std::stoi(str));
	 }

	 inline unsigned int to_value(const char *str, unsigned int &value) {
		return (value = (unsigned int) std::stoul(str));
	 }

	 inline short to_value(const char *str, short &value) {
		return (value = (short) std::stoi(str));
	 }

	 inline unsigned short to_value(const char *str, unsigned short &value) {
		return (value = (unsigned short) std::stoi(str));
	 }

	 inline long to_value(const char *str, long &value) {
		return (value = std::stol(str));
	 }

	 inline unsigned long to_value(const char *str, unsigned long &value) {
		return (value = std::stoul(str));
	 }

	inline float to_value(const char *str, float &value) {
		return (value = std::stof(str));
	 }

	inline double to_value(const char *str, double &value) {
		return (value = std::stod(str));
	 }

 }

