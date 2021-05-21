/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const Udjat::ModuleInfo moduleinfo {
		PACKAGE_NAME,
		"Basic agent state builder",
		PACKAGE_VERSION "." PACKAGE_RELEASE,
#ifdef PACKAGE_URL
		PACKAGE_URL,
#else
		"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
		PACKAGE_BUG_REPORT
#else
		""
#endif // PACKAGE_BUG_REPORT
	};

	std::shared_ptr<Abstract::State> get_default_state() {

		class DefaultState : public Abstract::State, Factory {
		public:

			DefaultState() : Abstract::State(""), Factory("state", &moduleinfo) {
			}

			~DefaultState() {
			}

			void parse(Abstract::Agent &agent, const pugi::xml_node &node) const override {

#ifdef DEBUG
				cout << "Parsing state 'default' for agent '" << agent.getName() << "'" << endl;
#endif // DEBUG

				agent.append_state(node);

			}

		};

		static shared_ptr<Abstract::State> state(new DefaultState());
		return state;

	}

	/// @brief Activate an error state.
	void Abstract::Agent::failed(const char *summary, const std::exception &e) noexcept {

		error("{}: {}",summary,e.what());

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(make_shared<Abstract::State>(summary,e));

	}

	void Abstract::Agent::failed(const char *summary, int code) noexcept {

		error("{}: {}",summary,string(strerror(code)));

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(make_shared<Abstract::State>("error",Udjat::critical,summary,strerror(errno)));

	}

	/// @brief Set failed state from known exception
	void Abstract::Agent::failed(const char *summary, const char *body) noexcept {

		error("{}",summary);

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(make_shared<Abstract::State>("error",Udjat::critical,summary,body));

	}

	void Abstract::Agent::onChildStateChange() noexcept {

		try {

			// Compute my current state based on value.
			auto new_state = find_state();

			// Then check the children.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {

					if(child->state->getLevel() > new_state->getLevel()) {
						new_state = child->state;
					}
				}
			}

			activate(new_state);

		} catch(const std::exception &e) {

			error("Error '{}' switching state",e.what());
			this->state = make_shared<Abstract::State>("Error switching state",e);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");

		}


	}

	bool Abstract::Agent::activate(std::shared_ptr<State> state) noexcept {

		// It's an empty state? If yes replaces with the default one.
		if(!state)
			state = Abstract::Agent::find_state();

		// Return if it's the same.
		if(state == this->state)
			return false;

		info("Current state changes from '{}' to '{}'",
				this->state->getSummary(),
				state->getSummary()
			);

		Udjat::Level saved_state = this->state->getLevel();

		try {

			this->state->deactivate(*this);
			this->state = state;
			this->state->activate(*this);

			if(parent)
				parent->onChildStateChange();

		} catch(const std::exception &e) {

			error("Error '{}' switching state",e.what());
			this->state = make_shared<Abstract::State>("Error switching state",e);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");

		}

#ifdef DEBUG
		if(saved_state != this->state->getLevel()) {
			info("State has changed from '{}' to '{}'",saved_state,this->state->getLevel());
		}
#endif // DEBUG

		return (saved_state != this->state->getLevel());
	}

}
