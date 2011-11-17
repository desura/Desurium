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

#ifndef DESURA_CRYPTO_H
#define DESURA_CRYPTO_H
#ifdef _WIN32
#pragma once
#endif

#include "Common.h"
#include <string>

namespace UTIL
{
namespace CRYPTO
{
	//! Encrypet a string using aes
	//! 
	//! @param dest Dest buffer (will allocate, make sure you delete it)
	//! @param str Input string
	//! @param key Input key
	//!
	void ciphertext(char** dest, const char* str, const char* key);

	//! Decrypet a string using aes
	//! 
	//! @param dest Dest buffer (will allocate, make sure you delete it)
	//! @param str Input string
	//! @param key Input key
	//!
	void deCiphertext(char** dest, const char* str, const char* key);

	//! Get the computers mac address. Although this is not part of the crypt lib. It is needed by it.
	//!
	//! @param macAdd Out buffer
	//! @param size Out buffer size
	//! @param adapterNum Which adapater to get
	//!
	void getAMacAddress(char* macAdd, uint32 size, uint32 adapterNum = 0);
}
}
#endif