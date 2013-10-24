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


#ifndef DESURA_BRANCHINSTALLINFO_H
#define DESURA_BRANCHINSTALLINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemInfoI.h"

class InsCheck;

namespace sqlite3x
{
	class sqlite3_connection;
}

class WildcardManager;

namespace UserCore
{
namespace Item
{

class ItemInfo;

class ExeInfo : public Misc::ExeInfoI
{
public:
	ExeInfo(const char* name)
	:	m_szName(name),
		m_uiRank(-1)
	{}

	virtual const char* getName()
	{
		return m_szName.c_str();
	}

	virtual const char* getExe()
	{
		return m_szExe.c_str();
	}

	virtual const char* getExeArgs()
	{
		return m_szExeArgs.c_str();
	}

	virtual const char* getUserArgs()
	{
		return m_szUserArgs.c_str();
	}

	void setExe(const char* exe)
	{
		if (!exe)
			m_szExe = "";
		else
			m_szExe = UTIL::FS::PathWithFile(exe).getFullPath();
	}

	void setUserArgs(const char* args)
	{
		m_szUserArgs = gcString(args);
	}

	gcString m_szExe;		//exe command
	gcString m_szExeArgs;	//command line args
	gcString m_szUserArgs;	//user command line args
	gcString m_szName;
	uint32 m_uiRank;
};

class ProcessResult
{
public:
	bool found;
	bool useCip;
	bool notFirst;
	gcString insCheck;
};

class BranchInstallInfo
{
public:
	BranchInstallInfo(uint32 biId, ItemInfo *itemInfo);
	~BranchInstallInfo();


	//! Removes this from the db
	//!
	//! @param db Sqlite db connection
	//!
	void deleteFromDb(sqlite3x::sqlite3_connection* db);

	//! Save regular changed vars to db
	//!
	//! @param db Sqlite db connection
	//!
	void saveDb(sqlite3x::sqlite3_connection* db);

	//! Load vars from db
	//!
	//! @param db Sqlite db connection
	//!
	void loadDb(sqlite3x::sqlite3_connection* db);

	//! Load data for this from xml
	//!
	//! @param xmlNode Xml to get data from
	//!
	ProcessResult processSettings(tinyxml2::XMLNode* setNode, WildcardManager* pWildCard, bool reset, bool hasBroughtItem, const char* cipPath);


	void setLinkInfo(const char* szPath, const char* szExe, const char* szArgs);

	uint32 getExeCount(bool setActive);
	void setActiveExe(const char* name);
	UserCore::Item::Misc::ExeInfoI* getActiveExe();
	void getExeList(std::vector<UserCore::Item::Misc::ExeInfoI*> &list);


	const char* getInstallCheck();

	void updated();
	bool setInstalledMcf(MCFBuild build);
	void overideInstalledBuild(MCFBuild build);
	void resetInstalledMcf();

	bool processUpdateXml(tinyxml2::XMLNode* branch);

	bool hasInstalledMod();

	DesuraId getInstalledModId();
	const char* getPath();
	const char* getInsPrimary();
	void setInstalledModId(DesuraId id);
	const char* getInstalledVersion();
	uint64 getInstallSize();
	uint64 getDownloadSize();
	MCFBuild getLastInstalledBuild();
	MCFBuild getInstalledBuild();
	MCFBuild getNextUpdateBuild();
	void overideMcfBuild(MCFBuild build);

	uint32 getBiId();


	void setLinkInfo(const char* exe, const char* args);

protected:
	void launchExeHack();
	void processExes(tinyxml2::XMLNode* setNode, WildcardManager* pWildCard, bool useCip);

	virtual bool isValidFile(const gcString &strFile);

	void setPath(const char *path);
	void setInsCheck(const char *path);
	void setInsPrimary(const char* path);

	bool isInstalled();


	void extractInstallChecks(tinyxml2::XMLNode* icsNode, WildcardManager* pWildCard, std::vector<InsCheck> &vInsChecks);

	//Used for install items to convert install check to path relative to the dir the item is installed to
	bool updateInstallCheck(gcString &strCheckRes, const gcString &strPath);
	void UpdateInstallCheckList(const std::vector<InsCheck> &vInsChecks, WildcardManager* pWildCard);

private:
	gcString m_szPath;
	gcString m_szInsCheck;	//install check
	gcString m_szInsPrim;
	gcString m_szInsVersion;

	gcString m_szActiveExe;

	std::vector<ExeInfo*> m_vExeList;
	std::vector<gcString> m_vInstallChecks;

	MCFBuild m_NextBuild;	//next build
	MCFBuild m_INBuild;		//installed build
	MCFBuild m_LastBuild;	//last installed build

	uint64 m_uiInstallSize;
	uint64 m_uiDownloadSize;

	DesuraId m_ItemId;
	DesuraId m_iInstalledMod;

	uint32 m_BiId;

	ItemInfo* m_pItem;
};


inline const char* BranchInstallInfo::getInstallCheck()
{
	return m_szInsCheck.c_str();
}

inline bool BranchInstallInfo::hasInstalledMod()
{
	return m_iInstalledMod.isOk();
}

inline DesuraId BranchInstallInfo::getInstalledModId()
{
	return m_iInstalledMod;
}

inline const char* BranchInstallInfo::getPath()		
{
	return m_szPath.c_str();
}

inline const char* BranchInstallInfo::getInsPrimary()		
{
	return m_szInsPrim.c_str();
}

inline void BranchInstallInfo::setInstalledModId(DesuraId id)
{
	m_iInstalledMod = id;
}

inline const char* BranchInstallInfo::getInstalledVersion()	
{
	return m_szInsVersion.c_str();
}

inline uint64 BranchInstallInfo::getInstallSize()
{
	return m_uiInstallSize;
}

inline uint64 BranchInstallInfo::getDownloadSize()
{
	return m_uiDownloadSize;
}

inline MCFBuild BranchInstallInfo::getLastInstalledBuild()
{
	return m_LastBuild;
}

inline MCFBuild BranchInstallInfo::getInstalledBuild()
{
	return m_INBuild;
}

inline MCFBuild BranchInstallInfo::getNextUpdateBuild()
{
	return m_NextBuild;
}

inline void BranchInstallInfo::overideMcfBuild(MCFBuild build)
{
	m_INBuild = build;
}

inline uint32 BranchInstallInfo::getBiId()
{
	return m_BiId;
}

}
}

#endif //DESURA_BRANCHINSTALLINFO_H
