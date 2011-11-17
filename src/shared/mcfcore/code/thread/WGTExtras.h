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

#ifndef DESURA_WTEXTRAS_H
#define DESURA_WTEXTRAS_H
#ifdef _WIN32
#pragma once
#endif


namespace MCFCore
{
namespace Thread
{
//! Misc items for MCFCore Threads
namespace Misc
{


//! Containter class for download block information
class WGTBlock
{
public:
	WGTBlock()
	{
		buff = NULL;
		webOffset = 0;
		fileOffset = 0;
		size = 0;
		crc = 0;
		dlsize = 0;

		file = NULL;
		index = -1;
	}

	~WGTBlock()
	{
		safe_delete(buff);
	}

	uint64 webOffset;
	uint64 fileOffset;
	char* buff;
	uint32 size;
	uint32 dlsize;
	uint32 crc;


	MCFCore::MCFFile* file;
	uint32 index;

	gcString provider;
};

class WGTSuperBlock
{
public:
	WGTSuperBlock()
	{
		size = 0;
		done = 0;
		offset = 0;
	}

	uint64 offset;

	size_t size;
	size_t done;

	std::deque<WGTBlock*> vBlockList;
	::Thread::Mutex m_Lock;
};


}
}
}


#endif //DESURA_WTEXTRAS_H
