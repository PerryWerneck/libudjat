#ifndef SERVICE_H_INCLUDED

	#define SERVICE_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <ctime>
	#include <functional>

#ifndef _WIN32
	#include <poll.h>
#endif // !_WIN32

	namespace Udjat {

		namespace Service {

			class Controller;

			enum Event : short {
#ifdef _WIN32
				// https://msdn.microsoft.com/en-us/library/windows/desktop/ms740094(v=vs.85).aspx
				oninput         = POLLRDNORM,   		///< @brief There is data to read.
				onoutput        = POLLWRNORM,   		///< @brief Writing is now possible, though a write larger that the available space in a socket or pipe will still block
				onerror         = POLLERR,              ///< @brief Error condition
				onhangup        = POLLHUP,              ///< @brief Hang  up
#else
				oninput         = POLLIN,               ///< @brief There is data to read.
				onoutput        = POLLOUT,              ///< @brief Writing is now possible, though a write larger that the available space in a socket or pipe will still block
				onerror         = POLLERR,              ///< @brief Error condition
				onhangup        = POLLHUP,              ///< @brief Hang  up
#endif // WIN32
			};

			/// @brief Generic service module.
			class UDJAT_API Module {
			private:
				friend class Udjat::Service::Controller;

				/// @brief Module name.
				Quark name;

				/// @brief Is the module active?
				bool active;

			protected:

				virtual void onTimer(const time_t now);
				virtual void onEvent(const Event event);

				/// @brief Convenience method to associate timer with module.
				void setTimer(const time_t seconds, bool fire = false);

				/// @brief Convenience method to associate file/socket handler with this module.
				void setHandle(int fd, const Event event);

			public:
				Module(const Quark &name);
				virtual ~Module();

				virtual void start();
				virtual void reload();
				virtual void stop();

			};

			/// @brief Start/init the service controller; does nothing if it's already started.
			UDJAT_API void start() noexcept;

			/// @brief Run service main loop.
			UDJAT_API void run() noexcept;

			/// @brief Reload modules.
			UDJAT_API void reload();

			/// @brief Insert socket/file in the list of event sources.
			UDJAT_API void insert(void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

			/// @brief Insert socket/file in the list of event sources.
			UDJAT_API void insert(void *id, const char *name, int fd, const Event event, const std::function<bool(const Event event)> call);

			/// @brief Insert timer in the list of event sources.
			UDJAT_API void insert(void *id, time_t seconds, const std::function<bool(const time_t)> call);

			/// @brief Insert timer in the list of event sources.
			UDJAT_API void insert(void *id, const char *name, time_t seconds, const std::function<bool(const time_t)> call);

			/// @brief Insert and emit a timer.
			UDJAT_API void insert(void *id, const std::function<bool(const time_t)> call);

			/// @brief Insert and emit a timer.
			UDJAT_API void insert(void *id, const char *name, const std::function<bool(const time_t)> call);

			/// @brief Reset timer to a new value.
			UDJAT_API void reset(void *id, time_t seconds = 0, time_t value = 0);

			/// @brief Remove socket/file/timer/module from the list of event sources.
			UDJAT_API void remove(void *id);

		}

	}


#endif // SERVICE_H_INCLUDED
