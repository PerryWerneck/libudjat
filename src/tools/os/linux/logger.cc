
#include <config.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <mutex>
#include <unistd.h>
#include <syslog.h>
#include <udjat/tools/quark.h>

using namespace std;

namespace Udjat {

	//
	// @brief Log buffer
	//
	class Buffer : public std::string {
	public:
		Buffer(const Buffer &src) = delete;
		Buffer(const Buffer *src) = delete;

		Buffer() : std::string() {
		}

		bool push_back(int c) {

			if(c == EOF || c == '\n') {
				return true;
			}

			if(c >= ' ' || c == '\t') {
				std::string::push_back(c);
			}

			return false;
		}

	};

	static Buffer & getInstance(uint8_t id) {
		static Buffer instances[3];
		return instances[id];
	}

	//
	// Syslog writer
	//
	class SysWriter {
	private:
		SysWriter() {
			::openlog(Quark(Application::Name()).c_str(), LOG_PID, LOG_DAEMON);
		}

	public:
		~SysWriter() {
			::closelog();
		}

		SysWriter(const SysWriter &src) = delete;
		SysWriter(const SysWriter *src) = delete;

		static SysWriter & getInstance();

		void write(uint8_t id, const char *message) {
			static const int priority[] = {LOG_INFO,LOG_WARNING,LOG_ERR};
			syslog(priority[id],"%s",message);

		}

	};

	SysWriter & SysWriter::getInstance() {
		static SysWriter instance;
		return instance;
	}

	//
	// Logger
	//
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
			properties.name = Quark(attribute.as_string(this->properties.name)).c_str();
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Logger::redirect(const char *filename, bool console) {

		static std::mutex guard;

		class Writer : public std::basic_streambuf<char, std::char_traits<char> > {
		private:

			/// @brief The buffer id.
			uint8_t id;

			/// @brief Send output to console?
			bool console;

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

			void write(Buffer &buffer) {

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

				// Get '\t' position.
				auto pos = buffer.find("\t");

				// If enable write console output.
				if(console) {

					// Write current time.
					{
						time_t t = time(0);
						struct tm tm;
						localtime_r(&t,&tm);

						char buffer[80];
						memset(buffer,0,sizeof(buffer));

						size_t len = strftime(buffer, 79, "%x %X", &tm);

						if(len) {
							write(1,buffer);
						} else {
							write(1,"--/--/-- --:--:--");
						}

						write(1," ");
					}

					// Write module name & message
					char module[12];
					memset(module,' ',sizeof(module));

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

				{
					size_t length = buffer.size()+2;
					char tmp[length+1];
					size_t ix = 0;
					for(const char *ptr = buffer.c_str(); *ptr && ix < length; ptr++) {
						if(*ptr == '\t') {
							tmp[ix++] = ':';
							tmp[ix++] = ' ';
						} else if(*ptr < ' ') {
							tmp[ix++] = '?';
						} else {
							tmp[ix++] = *ptr;
						}
					}
					tmp[ix] = 0;

					SysWriter::getInstance().write(id,tmp);
				}

				buffer.erase();

			}

		protected:

			/// @brief Writes characters to the associated file from the put area
			int sync() override {
				return 0;
			}

			/// @brief Writes characters to the associated output sequence from the put area.
			int overflow(int c) override {

				lock_guard<std::mutex> lock(guard);
				Buffer & buffer = getInstance(id);

				if(buffer.push_back(c)) {
					write(buffer);
				}

				return c;

			}

		public:
			Writer(uint8_t i, bool c) : id(i), console(c) {
			}

			~Writer() {
			}

		};

		std::cout.rdbuf(new Writer(0,console));
		std::clog.rdbuf(new Writer(1,console));
		std::cerr.rdbuf(new Writer(2,console));

	}
	#pragma GCC diagnostic pop

}

