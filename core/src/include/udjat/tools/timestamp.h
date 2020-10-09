#ifndef TIMESTAMP_H_INCLUDED

	#define TIMESTAMP_H_INCLUDED

	#include <udjat/defs.h>
	#include <ctime>
	#include <string>

	namespace Udjat {

		/// @brief A time value (in seconds)
		class UDJAT_API TimeStamp {
		public:

			time_t value;

			TimeStamp(time_t t = time(nullptr)) : value(t) { }

			std::string to_string(const char *format = "%x %X") const noexcept;

			/// @brief Reseta valor com atraso.
			///
			/// @param seconds	Nº de segundos após o atual para setar.
			TimeStamp & reset(const uint32_t seconds) noexcept {
				value = time(nullptr) + seconds;
				return *this;
			}

			TimeStamp & operator=(const time_t t) noexcept {
				value = t;
				return *this;
			}

			bool operator<(time_t value) const noexcept {
				return this->value < value;
			}

			time_t operator-(time_t value) const noexcept {
				return this->value - value;
			}

		};

	}

namespace std {

	inline string to_string(const Udjat::TimeStamp &time) {
		return time.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::TimeStamp &time ) {
		return os << time.to_string();
	}

}


#endif // TIMESTAMP_H_INCLUDED
