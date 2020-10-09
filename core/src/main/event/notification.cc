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

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	bool Notification::hasListeners() noexcept {
		return false;
	}

	void Notification::emit() const noexcept {

#ifdef DEBUG
		cout << "\tEmiting notification '" << label.c_str() << "'" << endl;
#endif // DEBUG

	}

}

