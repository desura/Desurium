/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef DESURA_UTIL_MISC_H
#define DESURA_UTIL_MISC_H
#ifdef _WIN32
#pragma once
#endif

#include "util/UtilCallback.h"

//! Different type of image formats
enum IMAGE_TYPES
{
	IMAGE_VOID,	//!< Not an image
	IMAGE_GIF,	//!< Gif image
	IMAGE_JPG,	//!< Jpeg image
	IMAGE_PNG,	//!< Png image
};

#ifndef FHANDLE
#ifdef WIN32
	typedef void* FHANDLE;
#else
	typedef FILE* FHANDLE;
#endif
#endif

namespace UTIL
{
namespace MISC
{
	//! Checks to the file magic number to see if its an image
	//!
	//! @param header First 5 chars of the file header
	//! @return Type of image or void if not an image
	//! 
	IMAGE_TYPES isValidImage(const unsigned char header[5]);

	//! Get core count (not working!)
	//!
	//! @return core count
	//!
	uint8 getCoreCount();

	//! Convers string into long long
	//!
	//! @param ca Input string
	//! @return number
	//!
	int64 atoll(const char *ca );

	//! matches str in list
	//!
	//! @param str input string to match
	//! @param list of strings to check against
	//! @return True if string is in list, false if not
	//!
	bool matchList(const char* str, const char **list);

	//! Maths power function
	//!
	//! @param num Base
	//! @param power Power
	//! @return Base^Power
	//!
	uint32 pow(uint32 num, uint32 power);


	//! Convert one char from hex to dec
	//! 
	//! @param c Input hex char (0-9 A-F)
	//! @return Value of c
	//!
	uint32 xtod(char c);

	//! Convert a string from hex to dec
	//! 
	//! @param hex Input hex string (0-9 A-F)
	//! @return Value of string
	//!
	uint32 hextoDec(char *hex);

	//! Formats a size in bytes into a nice string
	//!
	//! @param size Size to format
	//!
	std::string niceSizeStr(uint64 size, bool capAtMib = false);

	//! Generates time remaning string
	//!
	//! @param hours Number of hours
	//! @param mins Number of mins
	//! @param rate Rate of progression
	//!
	std::string genTimeString(uint8 hours, uint8 mins, uint32 rate);

	//! Checks a string to see if its a web url
	//!
	//! @param str Input string
	//! @return True if url, false if not
	//!
	bool isWebURL(const char* str);

	//! Checks a string to see if a string is a emaill address
	//!
	//! @param email Input string
	//! @return True if url, false if not
	//!
	bool isValidEmail(const char* email);

	//! Gets the time diff from the input datetime to now
	//! 
	//! @param dateTime date time input in the format YYYYMMDDHHMMSS
	//! @param days out number of days between
	//! @param hours out number of hours between
	//!
	void getTimeDiffFromNow(const char* dateTime, uint32 &days, uint32 &hours, bool dtIsUTC = false);

	//! Hashes a buffer using md5. 
	//!
	//! @param buffer Input buffer
	//! @param size Buffer size
	//! @param out Buffer to save octal outbut into (16 bytes)
	//!
	void hashString(const char* str, uint32 size, char out[16]);

	//! Hashes a buffer using md5. 
	//!
	//! @param buffer Input buffer
	//! @param size Buffer size
	//! @return MD5 hash.
	//!
	std::string hashString(const char* str, uint32 size);

	//! Hashes a file using md5. 
	//!
	//! @param file Path to file
	//! @return MD5 hash.
	//!
	std::string hashFile(const std::string file);

	//! Hashes a file using md5. 
	//!
	//! @param file File native handle
	//! @param size Size of file to hash
	//! @return MD5 hash.
	//!
	std::string hashFile(FHANDLE file, uint64 size);

	//! Hash a string using RSHash
	//!
	//! @param str String to hash
	//! @return hash
	//!
	uint32 RSHash_CSTR(const std::string &str);
	uint32 RSHash_CSTR(const char* buff, size_t size);

	//! Hash a string using FNVHash
	//!
	//! @param str String to hash
	//! @return hash
	//!
	uint64 FNVHash64(const std::string &str);

	//! Hash a char using crc32
	//!
	//! @param byte to hash
	//! @param dwCrc32 last byte hash
	//! @return hash
	//!
	uint32 CRC32(const unsigned char byte, uint32 dwCrc32);

	//! Hash a buffer using crc32
	//!
	//! @param buff Buffer to hash
	//! @param len Buffer length
	//! @return hash
	//!
	uint32 CRC32(const unsigned char* buff, uint64 len);

#ifdef WIN32
	//! Converts an image to an ico file
	//! 
	//! @param imgPath Image path
	//! @param icoPath Icon save path
	//! @return True if succeceded
	//!
	bool convertToIco(const std::string &imgPath, const std::string &icoPath);

	//! Converts an image to a png file
	//!
	//! @param imgPath Image path
	//! @param icoPath Icon save path
	//! @param thumbnailSize Resize to thumbnailSize x thumbnailSize keep same width/height ratio
	//! @return True if succeceded
	//!
	bool convertToPng(const std::string &imgPath, const std::string &icoPath, size_t thumbnailSize = 0);

	void unloadImgLib();
	bool loadImgLib();
#endif

	std::string SHA1(const unsigned char* buff, uint32 len);


	class BZ2WorkerData;


	enum
	{
		BZ2_COMPRESS,
		BZ2_DECOMPRESS,
	};

	class BZ2Worker
	{
	public:
		BZ2Worker(uint32 type);
		~BZ2Worker();

		bool isInit(int &res);

		void doWork();

		void read(char* buff, size_t &size);
		void write(const char* buff, size_t size, bool end);

		uint32 getReadSize();
		int32 getLastStatus();

		template <typename F>
		void write(const char* buff, size_t size, F &f)
		{
			UTIL::CB::TemplateCallback<F> c(f);
			writeCB(buff, size, &c);
		}

		template <typename F>
		void write(const char* buff, size_t size, const F &f)
		{
			UTIL::CB::TemplateCallback<const F> c(f);
			writeCB(buff, size, &c);
		}

		//! Writes some data, does some work and call the callback if there is data to read
		//! Need to call end to end the stream
		//!
		void writeCB(const char* buff, size_t size, UTIL::CB::CallbackI* callback);


		template <typename F>
		void end(F &f)
		{
			UTIL::CB::TemplateCallback<F> c(f);
			endCB(&c);
		}

		template <typename F>
		void end(const F &f)
		{
			UTIL::CB::TemplateCallback<const F> c(f);
			endCB(&c);
		}

		//! Ends the stream and calls the callback with remaining read data
		//!
		void endCB(UTIL::CB::CallbackI* callback);


	private:
		BZ2WorkerData* m_pData;
	};


	class BufferData;

	class Buffer
	{
	public:
		Buffer(size_t size, bool zero = false);
		~Buffer();

		void resize(size_t newSize);
		void zero();

		size_t size();
		char* data();

		operator char*();

	private:
		BufferData* m_pData;
	};
}
}

#endif
