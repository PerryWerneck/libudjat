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
		PACKAGE_NAME,									// The module name.
		"State controller",			 					// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
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

		activate(Abstract::State::get(summary,e));

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

			/// @brief State needs activation?
			bool need_activation = true;

			// Compute my current state based on value.
			auto state = find_state();

			// Then check the children.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {
					if(child->getLevel() > state->getLevel()) {
						need_activation = false;	// A child state, was already activated.
						state = child->getState();
					}
				}
			}

			activate(state);

		} catch(const std::exception &e) {

			error("Error '{}' switching state",e.what());
			this->state.active = Abstract::State::get("Error switching state",e);
			this->state.activation = time(0);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->state.activation = time(0);

		}


	}

	bool Abstract::Agent::activate(std::shared_ptr<State> state) noexcept {

		// It's an empty state? If yes replaces with the default one.
		if(!state) {
			throw runtime_error("Activating an empty state");
		}

		// Return if it's the same.
		if(state == this->state.active)
			return false;

		info("Current state changes from '{}' to '{}'",
				this->state.active->getSummary(),
				state->getSummary()
			);

		Udjat::Level saved_level = this->getLevel();

		try {

			this->state.active->deactivate(*this);
			this->state.active = state;
			this->state.activation = time(0);
			this->state.active->activate(*this);

			if(parent)
				parent->onChildStateChange();

		} catch(const std::exception &e) {

			error("Error '{}' switching state",e.what());
			this->state.active = Abstract::State::get("Error switching state",e);
			this->state.activation = time(0);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->state.activation = time(0);

		}

#ifdef DEBUG
		if(saved_level != this->getLevel()) {
			info("State has changed from '{}' to '{}'",saved_level,this->getLevel());
		}
#endif // DEBUG

		return (saved_level != this->getLevel());
	}

}
