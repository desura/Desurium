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
#include "BranchInstallInfo.h"
#include "sqlite3x.hpp"
#include "sql/ItemInfoSql.h"

#include "ItemInfo.h"

#include "managers/WildcardManager.h"
#include "XMLMacros.h"

namespace UM = UserCore::Item;

class InsCheck
{
public:
	InsCheck(const char *c, const char *p)
	{
		check = gcString(c);
		path = gcString(p);
	}

	gcString check;
	gcString path;
};

namespace UserCore
{
namespace Item
{


BranchInstallInfo::BranchInstallInfo(uint32 biId, ItemInfo *itemInfo)
{
	m_uiInstallSize = 0;
	m_uiDownloadSize = 0;

	m_BiId = biId;
	m_ItemId = itemInfo->getId();
	m_pItem = itemInfo;
}

BranchInstallInfo::~BranchInstallInfo()
{
	safe_delete(m_vExeList);
}

void BranchInstallInfo::deleteFromDb(sqlite3x::sqlite3_connection* db)
{
	try
	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE FROM installinfo WHERE itemid=? AND biid=?;");
		cmd.bind(1, (long long int)m_ItemId.toInt64());
		cmd.bind(2, (int)m_BiId);
		cmd.executenonquery();
	}
	catch (...)
	{
	}
}

void BranchInstallInfo::saveDb(sqlite3x::sqlite3_connection* db)
{
	sqlite3x::sqlite3_command cmd(*db, "REPLACE INTO installinfo VALUES (?,?,?,?, ?,?,?,?);");

	cmd.bind(1, (long long int)m_ItemId.toInt64());
	cmd.bind(2, (int)m_BiId);
	cmd.bind(3, UTIL::OS::getRelativePath(m_szPath));
	cmd.bind(4, UTIL::OS::getRelativePath(m_szInsCheck));
	cmd.bind(5, UTIL::OS::getRelativePath(m_szInsPrim));
	cmd.bind(6, (long long int)m_iInstalledMod.toInt64());
	cmd.bind(7, (int)m_INBuild);
	cmd.bind(8, (int)m_LastBuild);

	cmd.executenonquery();


	for (size_t x=0; x<m_vExeList.size(); x++)
	{
		ExeInfo *ei = m_vExeList[x];

		sqlite3x::sqlite3_command cmd(*db, "REPLACE INTO exe VALUES (?,?,?,?,?,?,?);");

		cmd.bind(1, (long long int)m_ItemId.toInt64());
		cmd.bind(2, (int)m_BiId);
		cmd.bind(3, ei->m_szName);
		cmd.bind(4, UTIL::OS::getRelativePath(ei->m_szExe));
		cmd.bind(5, ei->m_szExeArgs);
		cmd.bind(6, ei->m_szUserArgs);
		cmd.bind(7, (int)ei->m_uiRank);

		cmd.executenonquery();
	}
}

void BranchInstallInfo::loadDb(sqlite3x::sqlite3_connection* db)
{
	if (!db)
		return;

	{
		sqlite3x::sqlite3_command cmd(*db, "SELECT * FROM installinfo WHERE itemid=? AND biid=?;");
		cmd.bind(1, (long long int)m_ItemId.toInt64());
		cmd.bind(2, (int)m_BiId);
	
		sqlite3x::sqlite3_reader reader = cmd.executereader();
		reader.read();

		m_szPath		= UTIL::OS::getAbsPath(reader.getstring(2)); //install path
		m_szInsCheck	= UTIL::OS::getAbsPath(reader.getstring(3)); //install check
		m_szInsPrim		= UTIL::OS::getAbsPath(reader.getstring(4)); //install primary

		m_iInstalledMod = DesuraId(reader.getint64(5));
		m_INBuild		= MCFBuild::BuildFromInt(reader.getint(6));
		m_LastBuild		= MCFBuild::BuildFromInt(reader.getint(7));
	}

	{
		sqlite3x::sqlite3_command cmd(*db, "SELECT * FROM exe WHERE itemid=? AND biid=?;");
		cmd.bind(1, (long long int)m_ItemId.toInt64());
		cmd.bind(2, (int)m_BiId);

		sqlite3x::sqlite3_reader reader = cmd.executereader();
	
		while (reader.read())
		{
			gcString name = reader.getstring(2);
			gcString exe = UTIL::OS::getAbsPath(reader.getstring(3));
			gcString exeargs = reader.getstring(4);
			gcString userargs = reader.getstring(5);
			uint32 rank = reader.getint(6);

			ExeInfo *ei = NULL;

			for (size_t x=0; x<m_vExeList.size(); x++)
			{
				if (m_vExeList[x]->m_szName == name)
				{
					ei = m_vExeList[x];
					break;
				}
			}

			if (!ei)
			{
				ei = new ExeInfo(name.c_str());
				m_vExeList.push_back(ei);
			}

			ei->m_szExe = exe;
			ei->m_szExeArgs = exeargs;
			ei->m_szUserArgs = userargs;
			ei->m_uiRank = rank;
		}
	}
}

ProcessResult BranchInstallInfo::processSettings(TiXmlNode* setNode, WildcardManager* pWildCard, bool reset, bool hasBroughtItem, const char* cipPath)
{
	ProcessResult pr;
	pr.found = false;
	pr.useCip = false;
	pr.notFirst = false;

	TiXmlNode* icsNode = setNode->FirstChild("installlocations");
	
	if (icsNode)
	{
		std::vector<InsCheck*> insCheck;
		TiXmlNode* icNode = icsNode->FirstChild("installlocation");
		while (icNode)
		{
			gcString iCheck;
			gcString iPath;

			XML::GetChild("check", iCheck, icNode);
			XML::GetChild("path", iPath, icNode);

			if (iCheck.size() > 0 && iPath.size() > 0)
			{
				insCheck.push_back(new InsCheck(iCheck.c_str(), iPath.c_str()));

				if (hasBroughtItem)
					break;
			}

			icNode = icNode->NextSibling();
		}

		size_t size = insCheck.size();

		if (reset && size)
			size = 1;

		for (size_t x=0; x<size; x++)
		{
			char* CheckRes = NULL;
			try
			{
				pWildCard->constructPath(insCheck[x]->check.c_str(), &CheckRes);

				if (CheckRes && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(CheckRes)))
				{
					setInsCheck(CheckRes);
					pWildCard->updateInstallWildcard("INSTALL_PATH", insCheck[x]->path.c_str());

					pr.found = true;
					pr.notFirst = (x != 0);

					safe_delete(CheckRes);
					break;
				}
			}
			catch (gcException &e)
			{
			}

			safe_delete(CheckRes);
		}

		if (!pr.found && insCheck.size()>0)
		{
			if (cipPath)
			{
				pWildCard->updateInstallWildcard("INSTALL_PATH", cipPath);
				pr.useCip = true;
			}
			else
			{
				char* CheckRes = NULL;
				try
				{
					pWildCard->constructPath(insCheck[0]->check.c_str(), &CheckRes);

					setInsCheck(CheckRes);
					pWildCard->updateInstallWildcard("INSTALL_PATH", insCheck[0]->path.c_str());
				}
				catch (gcException)
				{
				}

				safe_delete(CheckRes);
			}
		}

		safe_delete(insCheck);


		char *iPathRes = NULL;
		char* insPrim = NULL;
		char* insPrimRes = NULL;
		
		XML::GetChild("installprimary", insPrim, setNode);

		try
		{
			pWildCard->constructPath("%INSTALL_PATH%", &iPathRes);
			if (iPathRes)
				setPath(iPathRes);
			else
				Warning(gcString("ItemInfo: Install path for {0} is NULL.\n"));
		}
		catch (gcException &)
		{
		}

		try
		{
			pWildCard->constructPath(insPrim, &insPrimRes);
			if (insPrimRes)
				setInsPrimary(insPrimRes);
			else
				setInsPrimary("");
		}
		catch (gcException &)
		{
			setInsPrimary("");
		}

		safe_delete(iPathRes);
		safe_delete(insPrim);
		safe_delete(insPrimRes);

		processExes(setNode, pWildCard, pr.useCip);
		launchExeHack();
	}

	pr.insCheck = m_szInsCheck;
	return pr;
}


UserCore::Item::Misc::ExeInfoI* BranchInstallInfo::getActiveExe()
{
	UserCore::Item::Misc::ExeInfoI* ei = NULL;

	for (size_t x=0; x<m_vExeList.size(); x++)
	{
		if (m_szActiveExe == m_vExeList[x]->getName())
		{
			ei = m_vExeList[x];
			break;
		}
	}

	if (!ei)
	{
		uint32 index = 0;
		uint32 rank = -1;

		for (size_t x=0; x<m_vExeList.size(); x++)
		{
			if (m_vExeList[x]->m_uiRank > rank)
			{
				index = x;
				rank = m_vExeList[x]->m_uiRank;
			}
		}

		ei = m_vExeList[index];
	}

	return ei;
}

void BranchInstallInfo::setActiveExe(const char* name)
{
	gcString n(name);

	for (size_t x=0; x<m_vExeList.size(); x++)
	{
		if (n == m_vExeList[x]->getName())
		{
			m_szActiveExe = name;
			return;
		}
	}

	Warning(gcString("Failed to set active exe to [{0}]\n", name));
}

void BranchInstallInfo::getExeList(std::vector<UserCore::Item::Misc::ExeInfoI*> &list)
{
	for (size_t x=0; x<m_vExeList.size(); x++)
	{
		if (UTIL::FS::isValidFile(m_vExeList[x]->getExe()))
			list.push_back(m_vExeList[x]);
	}

	if (list.size() == 0 && m_vExeList.size() > 0)
		list.push_back(m_vExeList[0]);

	std::sort(list.begin(), list.end(), [](UserCore::Item::Misc::ExeInfoI* a, UserCore::Item::Misc::ExeInfoI* b) -> bool
	{
		return dynamic_cast<ExeInfo*>(a)->m_uiRank < dynamic_cast<ExeInfo*>(b)->m_uiRank;
	});
}

uint32 BranchInstallInfo::getExeCount(bool setActive)
{
	uint32 count = 0;
	uint32 first = -1;

	for (size_t x=0; x<m_vExeList.size(); x++)
	{
		if (UTIL::FS::isValidFile(m_vExeList[x]->getExe()))
		{
			count++;

			if (first == UINT_MAX)
				first = x;
		}
	}

	if (setActive && count == 1 && first != UINT_MAX)
		setActiveExe(m_vExeList[first]->getName());

	//should always have 1 exe
	if (count == 0)
		count = 1;

	return count;
}

void BranchInstallInfo::setPath(const char *path)		
{
	if (!path)
	{
		m_szPath = gcString();
	}
	else if (!(m_pItem->getStatus() & UM::ItemInfoI::STATUS_INSTALLED))
	{
		m_szPath = UTIL::FS::PathWithFile(path).getFullPath();
	}
}

void BranchInstallInfo::setInsPrimary(const char* path)
{
	if (!path)
	{
		m_szInsPrim = gcString();
	}
	else if (!(m_pItem->getStatus() & UM::ItemInfoI::STATUS_INSTALLED))
	{
		m_szInsPrim = UTIL::FS::PathWithFile(path).getFullPath();
	}	
}

//only can change this if it is not installed
void BranchInstallInfo::setInsCheck(const char* path)	
{
	if (!path)
	{
		m_szInsCheck = gcString();
	}
	else if (!(m_pItem->getStatus() & UM::ItemInfoI::STATUS_INSTALLED))
	{
		m_szInsCheck = UTIL::FS::PathWithFile(path).getFullPath();
	}
}

void BranchInstallInfo::launchExeHack()
{
#ifdef WIN32
	//deus ex
	if (m_ItemId.getType() == DesuraId::TYPE_GAME && m_ItemId.getItem() == 14)
	{
		std::string steam = UTIL::OS::getConfigValue("HKEY_CURRENT_USER\\SOFTWARE\\Valve\\Steam\\SteamPath");
		gcString steamPath = UTIL::FS::Path(steam, "", false).getFullPath();

		if (steamPath.size() > 0)
		{
			for (size_t x=0; x<m_vExeList.size(); x++)
			{
				ExeInfo* ei = m_vExeList[x];

				if (ei->m_szExe.find(steamPath) != std::string::npos)
				{
					ei->m_szExe = gcString("{0}\\steam.exe", steamPath);

					if (ei->m_szName == "Play")
						ei->m_szExeArgs = gcString("-applaunch 6910");
					else
						ei->m_szExeArgs += gcString("-applaunch 6910");		
				}
			}
		}
	}
#endif
}

void BranchInstallInfo::processExes(TiXmlNode* setNode, WildcardManager* pWildCard, bool useCip)
{
	uint32 rank = 0;

	XML::for_each_child("execute", setNode->FirstChild("executes"), [&](TiXmlElement* exe)
	{
		gcString ePath;
		gcString args;
		gcString name;

		XML::GetChild("name", name, exe);
		XML::GetChild("exe", ePath, exe);
		XML::GetChild("args", args, exe);

		if (ePath.size() == 0)
			return;

		if (name.size() == 0)
			name = "Play";

		char* ePathRes = NULL;
		char* argsRes = NULL;

		ExeInfo* ei = NULL;

		for (size_t x=0; x<m_vExeList.size(); x++)
		{
			if (m_vExeList[x]->m_szName == name)
			{
				ei = m_vExeList[x];
				break;
			}
		}

		if (!ei)
		{
			ei = new ExeInfo(name.c_str());
			m_vExeList.push_back(ei);
		}

		ei->m_uiRank = rank;

		try
		{
			pWildCard->constructPath(ePath.c_str(), &ePathRes);
			if (ePathRes)
			{
				ei->setExe(ePathRes);

				//cip needs an install check
				if (useCip)
					setInsCheck(ePathRes);
			}
			else
			{
				ei->setExe(ePath.c_str());
			}
		}
		catch (gcException &)
		{
			ei->setExe(ePath.c_str());
		}

		try
		{
			pWildCard->constructPath(args.c_str(), &argsRes, false);
			ei->m_szExeArgs = argsRes ? argsRes : args;
		}
		catch (gcException &)
		{
			ei->m_szExeArgs = args;
		}

		safe_delete(ePathRes);
		safe_delete(argsRes);
		rank++;
	});
}


void BranchInstallInfo::updated()
{
	if (m_NextBuild > m_INBuild)
	{
		m_LastBuild = m_INBuild;
		m_INBuild = m_NextBuild;
	}
}

bool BranchInstallInfo::setInstalledMcf(MCFBuild build)
{
	m_LastBuild = m_INBuild;
	m_INBuild = build;

	if (m_NextBuild == m_INBuild)
	{
		m_NextBuild = MCFBuild();
		return true;
	}

	return false;
}

void BranchInstallInfo::overideInstalledBuild(MCFBuild build)
{
	m_LastBuild = m_INBuild;
	m_INBuild = build;
}

void BranchInstallInfo::resetInstalledMcf()
{
	m_LastBuild = MCFBuild();
	m_INBuild = MCFBuild();
}

bool BranchInstallInfo::processUpdateXml(TiXmlNode* branch)
{
	TiXmlElement* mcfEl = branch->FirstChildElement("mcf");
	if (mcfEl)
	{
		const char* id = mcfEl->Attribute("id");
					
		if (id)
		{
			uint32 build = -1;
			XML::GetChild("build", build, mcfEl);
			m_NextBuild = MCFBuild::BuildFromInt(build);
		}
	}					

	return (m_NextBuild > m_INBuild);
}

void BranchInstallInfo::setLinkInfo(const char* exe, const char* args)
{
	if (m_vExeList.size() == 0)
		m_vExeList.push_back(new ExeInfo("Link"));

	ExeInfo* info = m_vExeList[0];

	info->setExe(exe);
	info->setUserArgs(args);
}

}
}
