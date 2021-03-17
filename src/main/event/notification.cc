/**
 * @file
 *
 * @brief
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/notification.h>
 #include <udjat/tools/threadpool.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Notification::Controller::Controller() {
	}

	Notification::Controller::~Controller() {
	}

	Notification::Controller & Notification::Controller::getInstance() {
		static Controller controller;
		return controller;
	}

	void Notification::Controller::insert(const std::function<void(const Notification &)> method) {
		lock_guard<std::recursive_mutex> lock(guard);
		methods.push_back(method);
	}

	void Notification::Controller::emit(const Notification & notification) noexcept {
		lock_guard<std::recursive_mutex> lock(guard);

		ThreadPool & threads = ThreadPool::getInstance();
		for(auto method : methods) {
			threads.push([notification, method]() {

#ifdef DEBUG
				cout << "\tEmiting notification '" << notification.label.c_str() << "'" << endl;
#endif // DEBUG

				method(notification);
			});
		}

	}

	bool Notification::hasListeners() noexcept {
		return !Controller::getInstance().empty();
	}

	void Notification::emit() const noexcept {
		Controller::getInstance().emit(*this);
	}

	void Notification::insert(std::function<void(const Notification &)> method) {
		Controller::getInstance().insert(method);
	}

}

