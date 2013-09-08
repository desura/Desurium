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

#ifndef DESURA_BZIP2_H
#define DESURA_BZIP2_H
#ifdef _WIN32
#pragma once
#endif

#include "bzlib.h"

namespace UTIL
{
	namespace BZIP
	{

	inline void BZ2CBuff( char* dest, uint32* destLen, char* source, uint32  sourceLen)
	{
		int res =  BZ2_bzBuffToBuffCompress(dest, destLen, source, sourceLen, 9, 0, 30);

		if (res != 0)
			throw gcException(ERR_BZ2CFAIL, res, gcString("Failed to compress, Bzip2 Library error: {0}", res));
	}

	inline void BZ2DBuff( char* dest, uint32* destLen, char* source, uint32  sourceLen)
	{
		int res = BZ2_bzBuffToBuffDecompress(dest, destLen, source, sourceLen, 0, 0);

		if (res != 0)
			throw gcException(ERR_BZ2DFAIL, res, gcString("Failed to decompress, Bzip2 Library error: {0}", res));
	}

	//Make sure you delete the return buff
	inline char* BZ2DBuff(uint32* destLen, char* source, uint32 sourceLen)
	{
		if (*destLen == 0)
			*destLen = sourceLen* 15;

		char* dest = new char[*destLen];

		while (true)
		{
			int res = BZ2_bzBuffToBuffDecompress(dest, destLen, source, sourceLen, 0, 0);

			if (res == 0)
			{
				break;
			}
			if (res == -8)
			{
				delete [] dest;
				*destLen *= 2;
				dest = new char[*destLen];
			}
			else
			{
				throw gcException(ERR_BZ2DFAIL, res, gcString("Failed to decompress, Bzip2 Library error: {0}", res));
			}
		}
	
		return dest;
	}


	//Make sure you delete the return buff
	inline char* BZ2CBuff(uint32* destLen, char* source, uint32 sourceLen)
	{
		if (!destLen)
			return NULL;

		if (*destLen == 0)
			*destLen = sourceLen* 15;

		char* dest = new char[*destLen];

		while (true)
		{
			int res = BZ2_bzBuffToBuffCompress(dest, destLen, source, sourceLen, 9, 0, 30);

			if (res == 0)
			{
				break;
			}
			if (res == -8)
			{
				delete [] dest;
				*destLen *= 2;
				dest = new char[*destLen];
			}
			else
			{
				throw gcException(ERR_BZ2DFAIL, res, gcString("Failed to compress, Bzip2 Library error: {0}", res));
			}
		}
	
		return dest;
	}



}
}

#endif //DESURA_BZIP2_H
