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

#include "Common.h"
#include "MD5Wrapper.h"
#include "third_party/md5.h"

#define BUFSIZE (10*1024)

namespace md5wrapper
{

std::string getHashFromString(std::string text)
{
	return MD5::CalcMD5((const unsigned char*)text.c_str(), text.size());
}

std::string getHashFromFile(std::string filename)
{
	FILE* fh = Safe::fopen(filename.c_str(), "rb");

	if (!fh)
		return "-1";

	std::string hash = MD5::CalcMD5(fh);
	fclose(fh);

	return hash;
}

std::string getHashFromBuffer(const unsigned char *buffer, uint32 size)
{
	return MD5::CalcMD5(buffer, size);
}

void getHashFromBuffer(const unsigned char *buffer, uint32 size, char unsigned out[16])
{
	MD5::CalcMD5(buffer, size, out);
}

#ifdef WIN32
std::string getHashFromFile(HANDLE file, uint64 size)
#else
std::string getHashFromFile(FILE* file, uint64 size)
#endif
{
	if (!file)
		return "-1";
 
	MD5 md5;
  	unsigned char buffer[BUFSIZE];


	//read the filecontent
	uint64 done = 0;
	uint32 buffSize = BUFSIZE;

	while (done < size)
   	{
		if ((size-done) < BUFSIZE)
			buffSize = (uint32)(size-done);
		else
			buffSize = BUFSIZE;

#ifdef WIN32
		DWORD dwRead = 0;
		ReadFile(file, buffer, buffSize, &dwRead, NULL);
#else
		uint32 dwRead = fread(buffer, 1, buffSize, file);
#endif

		if (dwRead != buffSize)
			return "-1";

		md5.update(buffer, buffSize);
		done += buffSize;
	}
	
	md5.finalize();
	return md5.getMd5();
}


}
