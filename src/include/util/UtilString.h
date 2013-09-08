/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_UTIL_STRING_H
#define DESURA_UTIL_STRING_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include "gcString.h"
#include "gcBuff.h"

#include "util/UtilCallback.h"

#ifdef WIN32
typedef void* FHANDLE;
#endif

namespace UTIL
{
namespace STRING
{
	//! Zero a buffer
	//! 
	//! @param buff Buffer to zero
	//! @param size Size of buffer
	//!
	void zeroBuffer(char* buff, size_t size);

	//! Zero a wchar buffer
	//! 
	//! @param buff Buffer to zero
	//! @param size Size of buffer
	//!
	void zeroBuffer(wchar_t* str, size_t maxlen);

	//! Print buffer in hex to stdout
	//!
	//! @param buff Buffer to print
	//! @param size Buffer size
	//!
	void printBuffer(char* buff, size_t size);

	//! Split a url into host and path
	//!
	//! @param url Input url
	//! @param server String to save server into (out)
	//! @param serSize Size of server string
	//! @param path String to save path into
	//! @param pathSize Size of path string
	//!
	void splitURL(const char *url, char* server, size_t serSize, char* path, size_t pathSize);

	//! Gets the short name of a string
	//! 
	//! @param dest Out string
	//! @param size Out string size
	//! @param src Input string
	//!
	std::string getShortName(const std::string &src);

	//! Converts a arg string into a list of args
	//! 
	//! @param string Input string
	//! @param argv Output string vector
	//!
	void converToArgs(const std::string &string, std::vector<char*> &argv );

	//! Removes all .. / and \ from a file name
	//!
	//! @param file File to sanitize
	//!
	std::string sanitizeFileName(const std::string &file);

	//! Removes all .. and converts slash to common form
	//!
	//! @param filePath File path to sanitize (IN/OUT)
	//!
	std::string sanitizeFilePath(const std::string &filePath, char slash = DIRS_CHAR);

	//! Conver wstring to string
	//!
	//! @param widestring In wstring
	//! @return String
	//!
	std::string toStr(const std::wstring& widestring);

	//! Conver string to wstring
	//!
	//! @param utf8string In string
	//! @return WString
	//!
	std::wstring toWStr(const std::string& utf8string);


	//! Tokenizes an input string
	//! 
	//! @param str Input string
	//! @param tokens Output token list
	//! @param delimiter String to split on
	//!
	void tokenize(std::string str, std::vector<std::string>& tokens, std::string delimiter);


	//! Decodes a url
	//!
	//! @param inStr string to decode
	//! @return decoded string
	//!
	std::string urlDecode(const std::string& inStr);

	//! Encodes a url
	//!
	//! @param inStr string to encode
	//! @return encoded string
	//!
	std::string urlEncode(const std::string& inStr);



	//! Decode a base 64 string into a buffer
	//!
	//! @param encoded_string String to decode
	//! @param outLen Length of returned string
	//! @return Decoded string (must delete)
	//!
	unsigned char* base64_decode(const std::string &encoded_string, size_t &outlen);

	//! Encode a binary buffer into a base 64 string
	//!
	//! @param bytes Buffer to encode
	//! @param len Buffer length
	//! @return Encoded string
	//!
	std::string base64_encode(const char* bytes, size_t len);


	void base64_decodeCB(const std::string &encoded_string, UTIL::CB::CallbackI* callback);


	template <typename T>
	void base64_decode(const std::string &encoded_string, const T& callback)
	{
		UTIL::CB::TemplateCallback<const T> c(callback);
		base64_decodeCB(encoded_string, &c);
	}
	
	template <typename T>
	void base64_decode(const std::string &encoded_string, T& callback)
	{
		UTIL::CB::TemplateCallback<T> c(callback);
		base64_decodeCB(encoded_string, &c);
	}	

	//! Escapes the string
	//!
	//! @param in String to escape
	//! @return Escaped string
	//!
	std::string escape(const std::string &in);
}
}
#endif
