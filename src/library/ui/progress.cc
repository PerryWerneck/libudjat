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

 /**
  * @brief Implements the abstract progress bar.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/ui/console.h>
 #include <udjat/ui/progress.h>
 #include <private/dialog.h>
 #include <stdexcept>
 #include <memory>
 #include <mutex>

 /*
 using namespace std;

 namespace Udjat {


	Dialog::Progress::Factory * Dialog::Progress::Factory::instance = nullptr;
	
	Dialog::Progress::Progress() {
	}

	Dialog::Progress::~Progress() {
	}

	std::shared_ptr<Dialog::Progress> Dialog::Progress::getInstance() {
		return Factory::getInstance().ProgressFactory();
	}

	Dialog::Progress::Factory & Dialog::Progress::Factory::getInstance() {

		if(instance) {
			return *instance;
		}

		// Create default (text-mode) progress factory.
		class TextFactory : public Dialog::Progress::Factory {
		public:
			TextFactory() {
			}

			~TextFactory() override {
			}

			shared_ptr<Progress> ProgressFactory() const override {
				return make_shared<TextProgress>();
			}

		};

		static TextFactory textFactory;
		return textFactory;

	}

	Dialog::Progress::Factory::Factory() : parent{instance} {
		instance = this;
	}

	Dialog::Progress::Factory::~Factory() {
		if(instance != this) {
			throw logic_error("Progress factory not destroyed in the right order");
		}
		instance = parent;
	}

	Dialog::Progress & Dialog::Progress::item(const size_t, const size_t) {
		return *this;
	}

	void Dialog::Progress::set(uint64_t, uint64_t) {
	}

	Dialog::Progress & Dialog::Progress::url(const char *) {
		return *this;
	}

 }

 */
