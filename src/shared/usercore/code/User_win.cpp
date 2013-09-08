/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifdef WIN32
  #include <Wincrypt.h>
#endif

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

	gcString buff("00{0}{1}", username, passhash);

	buff[0]=username.size();
	buff[1]=passhash.size();


	gcString key = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\MachineGuid", true);

	DATA_BLOB secret;
	secret.pbData = (BYTE*)key.c_str();
	secret.cbData = key.size();

	DATA_BLOB db;
	db.pbData = (BYTE*)buff.c_str();
	db.cbData = buff.size();

	DATA_BLOB out;

	if (CryptProtectData(&db, NULL, &secret, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &out))
	{
		UTIL::FS::Path path(UTIL::OS::getLocalAppDataPath(), L"userinfo.upi", false);

		UTIL::FS::delFile(path.getFolderPath());
		UTIL::FS::recMakeFolder(path);

		try
		{
			UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_WRITE);

			size_t lenSize = out.cbData;
			char size[2];
			size[0] = (char)(lenSize&0xFF);
			size[1] = (char)((lenSize&0xFF00)>>8);

			fh.write(size, 2);
			fh.write((char*)out.pbData, lenSize);
		}
		catch (gcException &e)
		{
			Warning(gcString("Failed to open file to save login info: {0}.\n", e));
		}

		LocalFree(out.pbData);
	}
	else
	{
		Warning("Failed to encrypt login info for autosave.");
	}
}


void User::getLoginInfo(char** username, char** passhash)
{
	UTIL::FS::Path path(UTIL::OS::getLocalAppDataPath(), L"userinfo.upi", false);

	if (!UTIL::FS::isValidFile(path))
		throw gcException(ERR_BADPATH, "Unable to open password store.");


	UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_READ);

	unsigned char size[2];

	fh.read((char*)size, 2);

	char *fileText;
	uint32 readSize = 0;

	readSize = (uint32)size[0] + (((uint32)size[1])<<0x8); 

	fileText = new char[readSize+1];
	fh.read(fileText, readSize);

	fileText[readSize]='\0';

	DATA_BLOB db;

	db.pbData = (BYTE*)fileText;
	db.cbData = readSize;

	gcString key = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\MachineGuid", true);

	DATA_BLOB secret;
	secret.pbData = (BYTE*)key.c_str();
	secret.cbData = key.size();

	DATA_BLOB out;

	if (CryptUnprotectData(&db, NULL, &secret, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &out))
	{
		char* plaintext = (char*)out.pbData;

		uint32 uLen = (uint32)plaintext[0];
		uint32 pLen = (uint32)plaintext[1];

		safe_delete(*username);
		safe_delete(*passhash);

		if (Safe::strlen(plaintext, 2024) >= uLen+pLen+2)
		{
			*username = new char[uLen+1];
			*passhash = new char[pLen+1];

			Safe::strncpy(*username, uLen+1, plaintext+2, uLen);
			Safe::strncpy(*passhash, pLen+1, plaintext+2+uLen, pLen);

			(*username)[uLen] = '\0';
			(*passhash)[pLen] = '\0';
		}
	}

	safe_delete(fileText);
	LocalFree(out.pbData);
}

}
