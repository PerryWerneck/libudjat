#ifndef SERVICE_H_INCLUDED

	#define SERVICE_H_INCLUDED

	#include <udjat/defs.h>

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

				/// @brief Is the module active?
				bool active;

			protected:

				void setTimer(const time_t seconds, bool fire = false);
				void setEvent(int fd, const Event event);

				virtual void onTimer(const time_t now);
				virtual void onEvent(const Event event);

			public:
				Module();
				virtual ~Module();

				virtual void start();
				virtual void reload();
				virtual void stop();

			};

			/// @brief Run service main loop.
			UDJAT_API void run();

		}

	}


#endif // SERVICE_H_INCLUDED
