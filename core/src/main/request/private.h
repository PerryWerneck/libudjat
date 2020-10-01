#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/request.h>
	#include <udjat/tools/atom.h>
	#include <mutex>
	#include <unordered_map>

	using namespace std;

	namespace Udjat {

		class Request::Controller {
		private:

			static std::recursive_mutex guard;

			Controller();

			class Method {
			private:
				std::function<void(Request &request)> method;

			public:
				Method(std::function<void(Request &request)> m) : method(m) {}

				std::function<void(Request &request)> get() const noexcept {
					return this->method;
				}

			};

			std::unordered_map<Udjat::Atom, Method> methods;

		public:
			static Controller & getInstance();
			~Controller();

			void insert(const char *name, std::function<void(Request &request)> method);

			void call(Request &request);

		};


	}


#endif // PRIVATE_H_INCLUDED
