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

 #include "private.h"
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/url.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <iostream>

 using namespace std;

 static const char * expand(const char *value,const pugi::xml_node &node,const char *section);

 namespace Udjat {

	mutex Alert::Controller::guard;

	/// @brief Default alert (URL based).
	class DefaultAlert : public Alert {
	private:
		const char *url;
		const char *action;
		const char *payload;

	public:
		DefaultAlert(const pugi::xml_node &node) : Alert(node) {

			const char *section = node.attribute("settings-from").as_string("alert-defaults");

			url = ::expand(
					Attribute(node,"url")
						.as_string(
							Config::Value<string>(section,"url","")
						),
						node,
						section
					);

			payload = ::expand(
							node.child_value(),
							node,
							section
						);

			action = Quark(
							Attribute(node,"action")
							.as_string(
								Config::Value<string>(section,"action","get")
							)
						).c_str();

		}

		void activate(std::shared_ptr<Alert> alert, const std::function<void(std::string &str)> expander) override {

			class Activation : public Alert::Activation {
			private:
				string url;
				string action;
				string payload;

			public:
				Activation(std::shared_ptr<Alert> alert, const string &u, const char *a, const string &p) : Alert::Activation(alert), url(u), action(a), payload(p) {
				}

				void emit() const override {
					cout << "alerts\tEmitting '" << url << "'" << endl;
					auto response = URL(url.c_str()).call(action.c_str(),nullptr,payload.c_str());
					if(response->failed()) {
						throw runtime_error(to_string(response->getStatusCode()) + " " + response->getStatusMessage());
					}
				}

			};

			string url = this->url;
			string payload = this->payload;

			expander(url);
			expander(payload);

			insert(make_shared<Activation>(alert,url,this->action,payload));
		}

		/*
		void activate(std::shared_ptr<Alert> alert, const std::string &url, const std::string &payload) const {

			class Activation : public Alert::Activation {
			private:
				string url;
				string action;
				string payload;

			public:
				Activation(std::shared_ptr<Alert> alert, const string &u, const char *a, const string &p) : Alert::Activation(alert), url(u), action(a), payload(p) {
				}

				void emit() const override {
					cout << "alerts\tEmitting '" << url << "'" << endl;
					auto response = URL(url.c_str()).call(action.c_str(),nullptr,payload.c_str());
					if(response->failed()) {
						throw runtime_error(to_string(response->getStatusCode()) + " " + response->getStatusMessage());
					}
				}

			};

			insert(make_shared<Activation>(alert,url,this->action,payload));

		}

		void activate(const Abstract::Agent &agent, const Abstract::State &state, std::shared_ptr<Alert> alert) const override {

			string url{this->url};
			string payload{this->payload};

			agent.expand(url);
			agent.expand(payload);

			state.expand(url);
			state.expand(payload);

			activate(alert,url,payload);
		}
		*/

	};

	static const Udjat::ModuleInfo moduleinfo {
		PACKAGE_NAME,									// The module name.
		"Alert controller",			 					// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Alert::Controller::Controller() : Udjat::Factory("alert",&moduleinfo), Udjat::MainLoop::Service(&moduleinfo) {
		cout << "alerts\tInitializing" << endl;

		// Force creation of the default mainloop.
		MainLoop::getInstance();

	}

	Alert::Controller::~Controller() {
	}

	void Alert::Controller::stop() {
		cout << "alerts\tDeactivating active alerts" << endl;
		lock_guard<mutex> lock(guard);
		activations.clear();
		MainLoop::getInstance().remove(this);
	}

	void Alert::Controller::remove(const Alert *alert) {
		lock_guard<mutex> lock(guard);
		activations.remove_if([alert](const auto &activation){
			if(activation->alert().get() != alert)
				return false;
			return true;
		});
	}

	void Alert::Controller::reset(time_t seconds) noexcept {

		if(!seconds) {
			seconds = 1;
		}

		// Using threadpool because I cant change a timer from a timer callback.
		ThreadPool::getInstance().push([this,seconds]{
#ifdef DEBUG
				cout << "alert\tNext check scheduled to " << TimeStamp(time(0) + seconds) << endl;
#endif // DEBUG

			MainLoop &mainloop = MainLoop::getInstance();

			lock_guard<mutex> lock(guard);
			if(activations.empty()) {

				cout << "alerts\tStopping controller" << endl;
				mainloop.remove(this);

			} else if(!mainloop.reset(this,seconds*1000)) {

				cout << "alerts\tStarting controller" << endl;
				mainloop.insert(this,seconds*1000,[this]() {
					emit();
					return true;
				});

			}

		});

	}

	void Alert::Controller::refresh() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

		{
			lock_guard<mutex> lock(guard);
			for(auto activation : activations) {
				if(activation->timers.next > now) {
					next = min(next,activation->timers.next);
				} else {
					next = now+2;
				}
			}
		}

		reset(next-now);

	}

	void Alert::Controller::insert(const std::shared_ptr<Alert::Activation> activation) {

			{
				lock_guard<mutex> lock(guard);
				activations.push_back(activation);
			}

			emit();
	}

	void Alert::Controller::emit() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

		lock_guard<mutex> lock(guard);
		activations.remove_if([this,now,&next](auto activation){

			auto alert = activation->alert();

			if(!activation->timers.next) {
				// No alert or no next, remove from list.
				cout << "alerts\tAlert '" << alert->c_str() << "' was stopped" << endl;
				return true;
			}

			if(activation->timers.next <= now) {

				// Timer has expired
				activation->timers.next = (now + alert->timers.interval);

				if(activation->running) {

					clog << "alerts\tAlert '" << alert->c_str() << "' is running since " << TimeStamp(activation->running) << endl;

				} else {

					activation->running = time(0);

					ThreadPool::getInstance().push([this,activation]() {

						try {
							cout << "alerts\tEmitting '"
								<< activation->name() << "' ("
								<< (activation->count.success + activation->count.failed + 1)
								<< ")"
								<< endl;
							activation->timers.last = time(0);
							activation->emit();
							activation->success();
						} catch(const exception &e) {
							activation->failed();
							cerr << "alerts\tAlert '" << activation->name() << "': " << e.what() << " (" << activation->count.failed << " fail(s))" << endl;
						} catch(...) {
							activation->failed();
							cerr << "alerts\tAlert '" << activation->name() << "' has failed " << activation->count.failed << " time(s)" << endl;
						}
						activation->running = 0;

					});
				}
			}

			next = min(next,activation->timers.next);
			return false;
		});

		reset(next-now);

	}

	bool Alert::Controller::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		parent.append(make_shared<DefaultAlert>(node));
		return true;
	}

 }

 const char * expand(const char *value, const pugi::xml_node &node, const char *section) {

	string text{value};
	Udjat::expand(text, [node,section](const char *key) {

		auto attr = Udjat::Attribute(node,key,true);
		if(attr) {
			return (string) attr.as_string();
		}

		if(Udjat::Config::hasKey(section,key)) {
			return (string) Udjat::Config::Value<string>(section,key,"");
		}

		return string{"${}"};

	});

	return Udjat::Quark(text).c_str();

 }

