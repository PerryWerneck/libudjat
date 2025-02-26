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
#include <functional>
#include <mutex>
#include <udjat/tools/service.h>

namespace Udjat {

	/// @brief Abstract main loop.
	class UDJAT_API MainLoop {
	public:

		class Timer;
		class Handler;

		enum Type : uint8_t {
			Undefined,
			Pool,			///< @brief Internal mainloop.
			WinMsg,			///< @brief Win32 Object Window.
			GLib,			///< @brief GLib based mainloop.
			Custom
		};

	private:
		static MainLoop * instance;
		Type mtype;

	protected:

		class Message {
		public:
			Message();
			virtual ~Message();
			virtual void execute() = 0;
		};

		MainLoop(Type type);

		/// @brief Process message, delete it after processing.
		/// @param message The message to process.
		static void on_posted_message(Message *message) noexcept;

	public:

		MainLoop(const MainLoop &src) = delete;
		MainLoop(const MainLoop *src) = delete;

		virtual ~MainLoop();

		inline Type type() const noexcept {
			return mtype;
		}

		inline bool operator ==(const Type type) const noexcept {
			return this->mtype == type;
		}

		/// @brief Get default mainloop.
		static MainLoop & getInstance();

		/// @brief Is timer enabled?
		virtual bool enabled(const Timer *timer) const noexcept = 0;

		/// @brief Is Handler enabled?
		virtual bool enabled(const Handler *handler) const noexcept = 0;

		virtual void push_back(MainLoop::Timer *timer) = 0;
		virtual void remove(MainLoop::Timer *timer) = 0;

		/// @brief Timer has changed.
		/// @param timer The updated timer.
		/// @param from The original interval in milliseconds.
		/// @param to The new interval in milliseconds.
		virtual void changed(MainLoop::Timer *timer, unsigned long from, unsigned long to);

		/// @brief Handler has changed.
		/// @param handler The updated handler.
		virtual void changed(MainLoop::Handler *handler);

		virtual void push_back(MainLoop::Handler *handler) = 0;
		virtual void remove(MainLoop::Handler *handler) = 0;

		/// @brief Run mainloop.
		virtual int run() = 0;

		/// @brief Is the mainloop active?
		virtual bool active() const noexcept = 0;

		/// @brief Post message to mainloop.
		/// @param message Message to be posted, will be deleted after processing.
		virtual void post(Message *message) noexcept = 0;

		template <class T>
		inline void post(const T &value, const std::function<void(T &msg)> &call) {
			class Msg : public Message, private T {
			private:
				std::function<void(T &msg)> cbk;
			public:
				Msg(const T &value, const std::function<void(T &msg)> &call) : T{value}, cbk{call} {}
				virtual ~Msg() {
				}
				void execute() override {
					cbk(*this);
				}
			};

			post(new Msg{value,call});

		}

		inline operator bool() const noexcept {
			return active();
		}

		/// @brief Quit mainloop.
		virtual void quit() = 0;

		/// @brief Quit mainloop.
		virtual void quit(const char *message);

		/// @brief Wakeup main loop.
		virtual void wakeup() noexcept = 0;

		/// @brief Create timer for callback.
		/// @param interval	Timer interval on milliseconds.
		/// @param call Method when timer expires, timer will be deleted if it returns 'false'.
		/// @return Timer object.
		Timer * TimerFactory(unsigned long interval, const std::function<bool()> call);

	};

}
