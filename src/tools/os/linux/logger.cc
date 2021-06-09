
#include <config.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/timestamp.h>
#include <mutex>
#include <unistd.h>

using namespace std;

namespace Udjat {

	Logger::~Logger() {
	}

	Logger::Message & Logger::Message::append(const char *value) {

		size_t from = find("{");
		if(from == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		size_t to = find("}",from);
		if(to == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		replace(from,(to-from)+1,value);

		return *this;
	}

	Logger::Message & Logger::Message::append(const std::exception &e) {
		return append(e.what());
	}

	void Logger::set(const pugi::xml_node &node) {
		auto attribute = node.attribute("name");
		if(attribute)
			name = Quark(attribute.as_string(this->name)).c_str();
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Logger::redirect(const char *filename, bool console) {

		class Writer : public std::basic_streambuf<char, std::char_traits<char> > {
		private:

			/// @brief Send output to console?
			bool console;
			/// @brief Buffer contendo a linha de log.
			std::string buffer;

			/// @brief Mutex para serialização.
			std::recursive_mutex guard;

		protected:

			/// @brief Writes characters to the associated file from the put area
			int sync() override {
				return 0;
			}

			void write(int fd, const std::string &str) {

				size_t bytes = str.size();
				const char *ptr = str.c_str();

				while(bytes > 0) {

					ssize_t sz = ::write(fd,ptr,bytes);
					if(sz < 0)
						return;
					bytes -= sz;
					ptr += sz;

				}

			}

			void write() {

				lock_guard<std::recursive_mutex> lock(guard);

				// Remove spaces
				size_t len = buffer.size();
				while(len--) {

					if(isspace(buffer[len])) {
						buffer[len] = 0;
					} else {
						break;
					}
				}

				buffer.resize(strlen(buffer.c_str()));

				if(buffer.empty()) {
					return;
				}

				// Write Output file.
				TimeStamp tm;

				if(console) {
					char module[12];
					memset(module,' ',sizeof(module));

					write(1,tm.to_string("%x %X"));
					write(1," ");

					auto pos = buffer.find("\t");
					if(pos == string::npos) {
						module[sizeof(module)-1] = 0;
						write(1,module);
						write(1," ");
						write(1,buffer);
					} else {

						strncpy(module,buffer.c_str(),min(sizeof(module),pos));
						for(size_t ix = 0; ix < sizeof(module); ix++) {
							if(module[ix] < ' ') {
								module[ix] = ' ';
							}
						}
						module[sizeof(module)-1] = 0;
						write(1,module);
						write(1," ");
						write(1,buffer.c_str()+pos+1);
					}

					write(1,"\n");
					fsync(1);
				}

				buffer.erase();

			}

			/// @brief Writes characters to the associated output sequence from the put area.
			int overflow(int c) override {

				lock_guard<std::recursive_mutex> lock(guard);

				if(c == EOF || c == '\n' || c == '\r')
					write();
				else
					buffer += static_cast<char>(c);

				return c;
			}

		public:
			Writer(bool c) : console(c) {
			}

			~Writer() {
			}

		};

		std::clog.rdbuf(new Writer(console));
		std::cout.rdbuf(new Writer(console));
		std::cerr.rdbuf(new Writer(console));

	}
	#pragma GCC diagnostic pop

}

