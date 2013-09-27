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

#ifndef DESURA_TOOL_H
#define DESURA_TOOL_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseManager.h"
#define IGNORE_RESULT 0xFFFFFFFF

class OutValI;
class WildcardManager;

namespace sqlite3x
{
	class sqlite3_connection;
}

namespace UserCore
{

class ToolInfo : public BaseItem
{
public:
	ToolInfo(DesuraId id);
	~ToolInfo();

	DesuraId getId();

	const char* getExe();
	const char* getUrl();
	const char* getArgs();
	const char* getName();
	const char* getNameId();

	uint32 getDownloadSize();
	bool checkExpectedResult(uint32 res);


	virtual bool isInstalled();
	virtual bool isDownloaded();

	virtual void setExePath(const char* exe);
	void setInstalled(bool state = true);

	UTIL::FS::Path getPathFromUrl(const char* appDataPath);

	void checkFile(const char* appDataPath);

	const char* getResultString();

#ifdef NIX
	void overridePath(const char* exe);
#endif

protected:
	enum TOOL_FLAGS
	{
		TF_NONE = 0,
		TF_INSTALLED = 1<<0,
		TF_DOWNLOADED = 1<<1,
		TF_LINK = 1<<2,
	};

	void parseXml(tinyxml2::XMLNode* toolNode, WildcardManager* wildCardManager, const char* appDataPath);

	virtual void loadFromDb(sqlite3x::sqlite3_connection* db);
	virtual void saveToDb(sqlite3x::sqlite3_connection* db);

	friend class ToolManager;

	virtual bool checkExePath(const char* path, bool quick = false);

	bool processResultString();

	DesuraId m_ToolId;

	gcString m_szNameString;
	gcString m_szUrl;
	gcString m_szExe;
	gcString m_szArgs;
	gcString m_szHash;
	gcString m_szResult;

	uint32 m_uiFlags;
	uint32 m_uiDownloadSize;

	//result
	std::deque<OutValI*> m_vRPN;
};

}

#endif //DESURA_TOOL_H
