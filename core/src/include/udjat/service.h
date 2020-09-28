#ifndef SERVICE_H_INCLUDED

	#define SERVICE_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <mutex>

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

			/// @brief Run service main loop.
			DLL_PUBLIC void run();

		}

		namespace Abstract {

			class DLL_PUBLIC Timer {
			private:

				friend class Udjat::Service::Controller;

#ifdef _WIN32
				HANDLE hTimer;
#else
				time_t	next;
				time_t	interval;
#endif // _WIN32

			protected:
				virtual void onTimer() = 0;

			public:
				Timer(int seconds = 60, bool start = true);
				virtual ~Timer();

				void start(bool immediately = false);
				void stop();

			};

			class DLL_PUBLIC EventListener {
			private:

				friend class Udjat::Service::Controller;

				/// @brief Service name.
				const char *name;

				static std::mutex mtx;

			protected:

				/// @brief Is the service active?
				virtual bool isActive() const noexcept = 0;

				/// @brief Service start event.
				virtual void start() noexcept;

				/// @brief Service stop event.
				virtual void stop() noexcept;

				/// @brief Service reload event.
				virtual void reload() noexcept;

			public:
				EventListener(const EventListener *src);
				EventListener(const EventListener &src);
				EventListener(const char *name);
				virtual ~EventListener();

			};

		}


	}


#endif // SERVICE_H_INCLUDED
