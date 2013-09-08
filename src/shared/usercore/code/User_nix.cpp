/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>

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
#include "User.h"
#include "util/UtilLinux.h"



namespace UserCore
{

void User::saveLoginInfo()
{
	if (!m_pWebCore)
		return;

	gcString username(m_szUserName);
	gcString passhash(m_pWebCore->getIdCookie());

	if (passhash.size() > 255)
		return;

	if (username.size() > 255)
		return;

	size_t base = passhash.size() + username.size() + 2;
	size_t round = (base & (~63)) + 64;
	
	gcString buff("00{0}{1}", username, passhash);
	buff.resize(round, 0);
	
	buff[0]=username.size();
	buff[1]=passhash.size();

	gcString path = UTIL::OS::getAppDataPath(L"autologin");
	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(path.c_str()));

	try
	{
		UTIL::FS::FileHandle fh(path.c_str(), UTIL::FS::FILE_WRITE);

		char size[2];
		size[0] = (char)(base&0xFF);
		size[1] = (char)((base&0xFF00)>>8);

		fh.write(size, 2);
		fh.write(buff.c_str(), base);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to open file to save login info: {0}.\n", e));
	}
}


void User::getLoginInfo(char** username, char** passhash)
{
	gcString oldPath = UTIL::LIN::expandPath("~/.desura_autologin");
	gcString oldPath2 = UTIL::LIN::expandPath("~/.desura/.autologin");
	gcString path = UTIL::OS::getAppDataPath(L"autologin");
	
	if (UTIL::FS::isValidFile(oldPath.c_str()))
	{
		UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(path.c_str()));
		UTIL::FS::moveFile(oldPath.c_str(), path.c_str());
	}
	
	if (UTIL::FS::isValidFile(oldPath2.c_str()))
	{
		UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(path.c_str()));
		UTIL::FS::moveFile(oldPath2.c_str(), path.c_str());
	}
	
	if (!UTIL::FS::isValidFile(path.c_str()))
		throw gcException(ERR_BADPATH, "Unable to open password store.");

	UTIL::FS::FileHandle fh(path.c_str(), UTIL::FS::FILE_READ);

	unsigned char size[2];
	fh.read((char*)size, 2);

	size_t readSize = (uint32)size[0] + (((uint32)size[1])<<0x8); 
	size_t round = (readSize & (~63)) + 64;

	char *fileText = new char[round+1];
	
	memset(fileText, 0, round+1);
	fh.read(fileText, readSize);

	uint32 uLen = (uint32)fileText[0];
	uint32 pLen = (uint32)fileText[1];

	safe_delete(*username);
	safe_delete(*passhash);

	*username = new char[uLen+1];
	*passhash = new char[pLen+1];
	
	Safe::strncpy(*username, uLen+1, fileText+2, uLen);
	Safe::strncpy(*passhash, pLen+1, fileText+2+uLen, pLen);
	
	(*username)[uLen] = '\0';
	(*passhash)[pLen] = '\0';
	
	safe_delete(fileText);
}

}
