#ifndef UDJAT_FILE_H_INCLUDED

	#define UDJAT_FILE_H_INCLUDED

	#include <udjat/defs.h>

	namespace Udjat {

		/// @brief Memory mapped file.
		class UDJAT_API MemoryMappedFile {
		private:
			int fd;				///< @brief File handle.
			void * contents;	///< @brief File contents.
			size_t length;		///< @brief File length.

		public:
			MemoryMappedFile(const char *path);
			~MemoryMappedFile();

			inline const size_t size() const noexcept {
				return this->length;
			}

			inline const void * getPtr() const noexcept {
				return contents;
			}

		};

	}

#endif // UDJAT_FILE_H_INCLUDED
