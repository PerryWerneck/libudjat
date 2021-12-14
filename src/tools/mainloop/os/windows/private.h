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
 #include <winsock2.h>
 #include <windows.h>
 #include <udjat/defs.h>
 #include <udjat-internals.h>
 #include <udjat/tools/mainloop.h>

 #define WM_WAKE_UP				WM_USER+100
 #define WM_CHECK_TIMERS		WM_USER+101
 #define IDT_CHECK_TIMERS		1

 using namespace std;

 namespace Udjat {


 }