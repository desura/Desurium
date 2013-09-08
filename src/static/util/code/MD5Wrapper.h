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

#ifndef DESURA_MD5WRAPPER_H
#define DESURA_MD5WRAPPER_H
#ifdef _WIN32
#pragma once
#endif

#include <string>

namespace md5wrapper
{
	std::string getHashFromString(std::string text);

	std::string getHashFromFile(std::string filename);

	std::string getHashFromBuffer(const unsigned char *buffer, uint32 size);

	void getHashFromBuffer(const unsigned char *buffer, uint32 size, char unsigned out[16]);

#ifdef WIN32
	std::string getHashFromFile(HANDLE file, uint64 size);
#else
	std::string getHashFromFile(FILE* file, uint64 size);
#endif
}

#endif //DESURA_MD5WRAPPER_H


