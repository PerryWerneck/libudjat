/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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

 #define UDJAT_AUTOPTR_FUNC_NAME(TypeName) udjat_autoptr_cleanup_##TypeName
 #define udjat_autoptr(TypeName) TypeName __attribute__ ((__cleanup__(UDJAT_AUTOPTR_FUNC_NAME(TypeName))))
 #define udjat_auto_cleanup(TypeName) TypeName __attribute__ ((__cleanup__(UDJAT_AUTOPTR_FUNC_NAME(TypeName))))

