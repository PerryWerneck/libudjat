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
 #include <stdexcept>
 #include <memory>
 #include <mutex>
 #include <string>
 #include <udjat/tools/container.h>

 using namespace std;
 using namespace Udjat;

 class ProgressBar;
	
 class UDJAT_PRIVATE Controller : public Dialog::Progress::Factory, public Container<ProgressBar>, public UI::Console {
 public:
	Controller() = default;

	~Controller() override {
	}

	shared_ptr<Dialog::Progress> ProgressFactory() const override;

 };

 /// @brief Text mode progress dialog.
 class UDJAT_PRIVATE ProgressBar : public Dialog::Progress {
 private:

	Controller *controller;
	size_t line = 0;
	string prefix;
	string text;
	uint64_t current = 0;
	uint64_t total = 0;

	void update(const Udjat::UI::Console::Foreground color = Udjat::UI::Console::White) const noexcept {
		lock_guard<mutex> lock((std::mutex &) *controller);
		controller->up(line).set(color);
		controller->progress(prefix.c_str(), text.c_str(), current, total);
		controller->set(Udjat::UI::Console::White).down(line);
	}
	
 public:
	ProgressBar(Controller *cntrl) : controller{cntrl} {
		*controller << "\n";
		controller->push_back(this);
		controller->for_each([this](ProgressBar &object) -> bool {
			object.line++;
			return false;
		});
	}

	~ProgressBar() override {
		controller->remove(this);		
		if(controller->empty()) {
			delete controller;
		}
	}

	Udjat::Dialog::Progress & step(const unsigned int current, const unsigned int total) noexcept override {
		char buffer[15];
		snprintf(buffer,14,"%03u/%03u",current, total);
		prefix = buffer;
		update();
		return *this;
	}
	
	Udjat::Dialog::Progress & set(uint64_t c, uint64_t t, bool) noexcept override {
		current = c;
		total = t;
		update();
		return *this;
	}

	Udjat::Dialog::Progress & url(const char *u) noexcept override {
		if(!u) {
			text.clear();
		} else {
			text = u;
		}
		return *this;
	}

	Udjat::Dialog::Progress & done(bool success) noexcept override {
		current = total;
		update(success ? Udjat::UI::Console::Green : Udjat::UI::Console::Red);
		return *this;
	}
	
 };

 shared_ptr<Dialog::Progress> Controller::ProgressFactory() const {
	return make_shared<ProgressBar>(const_cast<Controller *>(this));
 }

 namespace Udjat {

	Dialog::Progress::Factory * Dialog::Progress::Factory::instance = nullptr;
	
	Dialog::Progress::Progress() {
	}

	Dialog::Progress::~Progress() {
	}

	Dialog::Progress::Factory * Dialog::Progress::Factory::getInstance() {
		if(!instance) {
			new Controller();
		}
		return instance;
	}

	std::shared_ptr<Dialog::Progress> Dialog::Progress::getInstance() {
		return Factory::getInstance()->ProgressFactory();
	}

	Dialog::Progress::Factory::Factory() : parent{instance} {
		instance = this;
	}

	Dialog::Progress::Factory::~Factory() {
		if(instance != this) {
			Logger::String{"Progress factory not destroyed in the right order"}.error();
		}
		instance = parent;
	}

	Dialog::Progress & Dialog::Progress::show() noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::hide() noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::done(bool) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::title(const char *) noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::step(const unsigned int, const unsigned int) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::set(uint64_t, uint64_t, bool) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::url(const char *) noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::message(const char *) noexcept {
		return *this;
	}

 }
