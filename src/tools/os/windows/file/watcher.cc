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

 /**
  * @brief Implements windows file watcher.
  */

 // References:
 //
 // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstchangenotificationw
 //

 #include <config.h>
 #include <udjat/tools/file/watcher.h>
 #include <private/filewatcher.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {


	File::Watcher::Controller::Controller() {
		Logger::String{"Starting service"}.trace("file-watcher");
	}

	File::Watcher::Controller::~Controller() {

		std::lock_guard<std::mutex> lock(guard);
		handlers.remove_if([](Handler &) {
			return true;
		});

	}

	File::Watcher::Controller & File::Watcher::Controller::getInstance() {
		static File::Watcher::Controller instance;
		return instance;
	}

	void File::Watcher::Controller::insert(File::Watcher *watcher) {

		std::lock_guard<std::mutex> lock(guard);

		DWORD attr = GetFileAttributes(watcher->pathname);
		if(attr == INVALID_FILE_ATTRIBUTES) {
			throw Win32::Exception(watcher->pathname);
		}

		if(attr & FILE_ATTRIBUTE_DIRECTORY) {
			watch_directory(watcher);
		} else {
			watch_file(watcher);
		}

		Logger::String{"Watching '",watcher->pathname,"'"}.trace("file-watcher");

	}

	void File::Watcher::Controller::add_watch(File::Watcher *watcher, const char *path, BOOL bWatchSubtree, DWORD dwNotifyFilter) {

		HANDLE hNotify = FindFirstChangeNotification(path,bWatchSubtree,dwNotifyFilter);
		if(hNotify == INVALID_HANDLE_VALUE) {
			throw Win32::Exception(path);
		}

		handlers.emplace_back(watcher,dwNotifyFilter,hNotify);

	}

	void File::Watcher::Controller::watch_file(File::Watcher *watcher) {

		throw system_error(ENOTSUP,system_category(),"Unable to watch files on windows");

	}

	void File::Watcher::Controller::watch_directory(File::Watcher *watcher) {

		add_watch(watcher,watcher->pathname,FALSE,FILE_NOTIFY_CHANGE_FILE_NAME);
		add_watch(watcher,watcher->pathname,FALSE,FILE_NOTIFY_CHANGE_DIR_NAME);

	}

	void File::Watcher::Controller::remove(File::Watcher *watcher) {

		std::lock_guard<std::mutex> lock(guard);
		handlers.remove_if([this,watcher](Handler &handler) {
			return handler.file == watcher;
		});

	}

	void File::Watcher::Controller::Handler::handle(bool) {

		debug("Event on '",file->pathname,"'");

		switch(dwNotifyFilter) {
		case FILE_NOTIFY_CHANGE_DIR_NAME:	// File created or deleted.
			file->updated(Modified,"");
			break;

		case FILE_NOTIFY_CHANGE_FILE_NAME:	// File changed
			file->updated(Modified,"");
			break;

		}

		if(FindNextChangeNotification(hEvent) == FALSE) {
			Logger::String{Win32::Exception::format(file->pathname)}.error("file-watcher");
		}

	}

 }

