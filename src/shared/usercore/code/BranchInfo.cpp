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
#include "BranchInfo.h"

#include "sqlite3x.hpp"
#include "XMLMacros.h"

#include "BranchInstallInfo.h"

namespace UserCore
{
namespace Item
{

BranchInfo::BranchInfo(MCFBranch branchId, DesuraId itemId, BranchInstallInfo* bii, uint32 platformId)
{
	m_InstallInfo = bii;
	m_ItemId = itemId;
	m_uiBranchId = branchId;
	m_uiFlags = 0;

	if (platformId != 0)
	{
		switch (platformId)
		{
		default:
		case 100:
			m_uiFlags |= BF_WINDOWS_32;
			break;
		case 110:
			m_uiFlags |= BF_LINUX_32;
			break;
		case 120:
			m_uiFlags |= BF_LINUX_64;
			break;
		};
	}
}

BranchInfo::~BranchInfo()
{
}

bool BranchInfo::isWindows()
{
	//default to windows
	bool res = HasAnyFlags(m_uiFlags, BF_WINDOWS_32|BF_WINDOWS_64) || (!isLinux() && !isMacOsX());
	return res;
}

bool BranchInfo::isLinux()
{
	bool res = HasAnyFlags(m_uiFlags, BF_LINUX_32|BF_LINUX_64);
	return res;
}

bool BranchInfo::isMacOsX()
{
	return HasAnyFlags(m_uiFlags, BF_MACOSX);
}

bool BranchInfo::is32Bit()
{
	//default to 32 bit
	bool res = HasAnyFlags(m_uiFlags, BF_WINDOWS_32|BF_LINUX_32) || isMacOsX() || !is64Bit();
	return res;
}

bool BranchInfo::is64Bit()
{
	bool res = HasAnyFlags(m_uiFlags, BF_WINDOWS_64|BF_LINUX_64) || isMacOsX();
	return res;
}

bool BranchInfo::isAvaliable()
{
	return HasAnyFlags(m_uiFlags, BF_FREE|BF_ONACCOUNT|BF_DEMO|BF_TEST) && !(HasAnyFlags(m_uiFlags, BF_REGIONLOCK|BF_MEMBERLOCK) && !HasAnyFlags(m_uiFlags, BF_ONACCOUNT));
}

bool BranchInfo::isDownloadable()
{
	return isAvaliable() && !HasAnyFlags(m_uiFlags, BF_NORELEASES);
}

bool  BranchInfo::isPreOrder()
{
	return isAvaliable() && HasAllFlags(m_uiFlags, BF_PREORDER);
}

bool BranchInfo::hasAcceptedEula()
{
	return (m_szEulaUrl.size() == 0 || HasAnyFlags(m_uiFlags, BF_ACCEPTED_EULA));
}

bool BranchInfo::hasCDKey()
{
	return HasAllFlags(m_uiFlags, BF_CDKEY);
}

bool BranchInfo::isSteamGame()
{
	return HasAllFlags(m_uiFlags, BF_STEAMGAME);
}

void BranchInfo::acceptEula()
{
	m_uiFlags |= BF_ACCEPTED_EULA;
}

void BranchInfo::deleteFromDb(sqlite3x::sqlite3_connection* db)
{
	try
	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE FROM branchinfo WHERE branchid=? AND internalid=?;");
		cmd.bind(1, (int)m_uiBranchId);
		cmd.bind(2, (long long int)m_ItemId.toInt64());
		cmd.executenonquery();
	}
	catch (...)
	{
	}

	try
	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE FROM tools WHERE branchid=?;");
		cmd.bind(1, (int)m_uiBranchId);
		cmd.executenonquery();
	}
	catch (...)
	{
	}
}

void BranchInfo::saveDb(sqlite3x::sqlite3_connection* db)
{
	saveDbFull(db);
}

void BranchInfo::saveDbFull(sqlite3x::sqlite3_connection* db)
{
	{
		sqlite3x::sqlite3_command cmd(*db, "REPLACE INTO branchinfo VALUES (?,?,?,?,?, ?,?,?,?,?, ?,?);");

		cmd.bind(1, (int)m_uiBranchId);
		cmd.bind(2, (long long int)m_ItemId.toInt64());
		cmd.bind(3, m_szName);				//name
		cmd.bind(4, (int)m_uiFlags);		//flags
		cmd.bind(5, m_szEulaUrl);

		cmd.bind(6, m_szEulaDate);
		cmd.bind(7, m_szPreOrderDate);
		cmd.bind(8, encodeCDKey());
		cmd.bind(9, UTIL::OS::getRelativePath(m_szInstallScript));
		cmd.bind(10, (int)m_uiInstallScriptCRC);
		
		cmd.bind(11, (int)m_uiGlobalId);
		cmd.bind(12, (int)m_InstallInfo->getBiId());

		cmd.executenonquery();
	}

	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE from tools WHERE branchid=?;");
		cmd.bind(1, (int)m_uiBranchId);
		cmd.executenonquery();
	}

	{
		sqlite3x::sqlite3_command cmd(*db, "INSERT INTO tools VALUES (?,?);");

		for (size_t x=0; x<m_vToolList.size(); x++)
		{
			cmd.bind(1, (int)m_uiBranchId);
			cmd.bind(2, (long long int)m_vToolList[x].toInt64());
			cmd.executenonquery();
		}
	}
}

void BranchInfo::loadDb(sqlite3x::sqlite3_connection* db)
{
	if (!db)
		return;

	sqlite3x::sqlite3_command cmd(*db, "SELECT * FROM branchinfo WHERE branchid=? AND internalid=?;");
	cmd.bind(1, (int)m_uiBranchId);
	cmd.bind(2, (long long int)m_ItemId.toInt64());

	sqlite3x::sqlite3_reader reader = cmd.executereader();
	
	reader.read();

	m_szName		= gcString(reader.getstring(2));		//name
	m_uiFlags		= reader.getint(3);						//flags
	m_szEulaUrl		= reader.getstring(4);
	m_szEulaDate	= reader.getstring(5);
	m_szPreOrderDate = reader.getstring(6);
	decodeCDKey(reader.getstring(7));
	m_szInstallScript = UTIL::OS::getAbsPath(reader.getstring(8));
	m_uiInstallScriptCRC = reader.getint(9);
	m_uiGlobalId = MCFBranch::BranchFromInt(reader.getint(10), true);

	m_uiFlags &= ~BF_ONACCOUNT;

	{
		sqlite3x::sqlite3_command cmd(*db, "SELECT * FROM tools WHERE branchid=?;");
		cmd.bind(1, (int)m_uiBranchId);
		sqlite3x::sqlite3_reader reader = cmd.executereader();
	
		while (reader.read())
			m_vToolList.push_back(DesuraId(reader.getint64(1)));
	}
}


void BranchInfo::loadXmlData(TiXmlNode *xmlNode)
{
	XML::GetChild("name", m_szName, xmlNode);
	XML::GetChild("price", m_szCost, xmlNode);
	XML::GetChild("eula", m_szEulaUrl, xmlNode);

	TiXmlNode* eNode = xmlNode->FirstChild("eula");

	if (eNode && XML::isValidElement(eNode))
	{
		TiXmlElement *eEl = dynamic_cast<TiXmlElement*>(eNode);

		const char* date = eEl->Attribute("date");

		if (date && m_szEulaDate != date)
		{
			m_uiFlags &= ~BF_ACCEPTED_EULA; 
			m_szEulaDate = date;
		}
	}

	gcString preload;
	XML::GetChild("preload", preload, xmlNode);

	if (m_szPreOrderDate.size() > 0 && (preload.size() == 0 || preload == "0"))
	{
		m_szPreOrderDate = "";
		m_uiFlags &= ~BF_PREORDER;

		onBranchInfoChangedEvent();
	}
	else if (preload != "0")
	{
		m_szPreOrderDate = preload;
		m_uiFlags |= BF_PREORDER;

		onBranchInfoChangedEvent();
	}


	bool nameon = false;
	bool free = false;
	bool onaccount = false;
	bool regionlock = false;
	bool memberlock = false;
	bool demo = false;
	bool test = false;
	bool cdkey = false;
	gcString cdkeyType;

	XML::GetChild("nameon", nameon, xmlNode);
	XML::GetChild("free", free, xmlNode);
	XML::GetChild("onaccount", onaccount, xmlNode);
	XML::GetChild("regionlock", regionlock, xmlNode);
	XML::GetChild("inviteonly", memberlock, xmlNode);
	XML::GetChild("demo", demo, xmlNode);
	XML::GetChild("test", test, xmlNode);
	XML::GetChild("cdkey", cdkey, xmlNode);
	XML::GetAtt("type", cdkeyType, xmlNode->FirstChildElement("cdkey"));

	int32 global = -1;
	XML::GetChild("global", global, xmlNode);

	if (global != -1)
		m_uiGlobalId = MCFBranch::BranchFromInt(global, true);

	if (nameon)
		m_uiFlags |= BF_DISPLAY_NAME;

	if (free)
		m_uiFlags |= BF_FREE;

	if (onaccount)
		m_uiFlags |= BF_ONACCOUNT;

	if (regionlock)
		m_uiFlags |= BF_REGIONLOCK;

	if (memberlock)
		m_uiFlags |= BF_MEMBERLOCK;

	if (demo)
		m_uiFlags |= BF_DEMO;

	if (test)
		m_uiFlags |= BF_TEST;

	if (cdkey)
		m_uiFlags |= BF_CDKEY;

	if (cdkeyType == "steam")
		m_uiFlags |= BF_STEAMGAME;

	//no mcf no release
	TiXmlNode* mcfNode = xmlNode->FirstChild("mcf");
	if (!mcfNode)
	{
		m_uiFlags |= BF_NORELEASES;
	}
	else
	{
		int32 build = -1;
		XML::GetChild("build", build, mcfNode);

		m_uiLatestBuild = MCFBuild::BuildFromInt(build);
	}

	TiXmlNode* toolsNode = xmlNode->FirstChild("tools");

	if (toolsNode)
	{
		m_vToolList.clear();

		TiXmlElement* toolNode = toolsNode->FirstChildElement("tool");

		while (toolNode)
		{
			const char* id = toolNode->GetText();

			if (id)
				m_vToolList.push_back(DesuraId(id, "tools"));

			toolNode = toolNode->NextSiblingElement("tool");
		}
	}

	TiXmlElement* scriptNode = xmlNode->FirstChildElement("installscript");

	if (scriptNode)
		processInstallScript(scriptNode);
}

void BranchInfo::processInstallScript(TiXmlElement* scriptNode)
{
	int crc = 0;
	scriptNode->Attribute("crc", &crc);

	if (UTIL::FS::isValidFile(m_szInstallScript))
	{
		if (crc != 0 && m_uiInstallScriptCRC == (uint32)crc)
			return;
	}
	else
	{
		m_szInstallScript = UTIL::OS::getAppDataPath(gcWString(L"{0}\\{1}\\install_script.js", m_ItemId.getFolderPathExtension(), getBranchId()).c_str());
	}

	gcString base64 = scriptNode->GetText();

	try
	{
		UTIL::FS::recMakeFolder(UTIL::FS::Path(m_szInstallScript, "", true));
		UTIL::FS::FileHandle fh(m_szInstallScript.c_str(), UTIL::FS::FILE_WRITE);
		UTIL::STRING::base64_decode(base64, [&fh](const unsigned char* data, uint32 size) -> bool
		{
			fh.write((const char*)data, size);
			return true;
		});

		m_uiInstallScriptCRC = crc;
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to save install script for {0} branch {1}: {2}\n", m_ItemId.toInt64(), m_uiBranchId, e));
		m_szInstallScript = "";
	}
}

void BranchInfo::getToolList(std::vector<DesuraId> &toolList)
{
	toolList = m_vToolList;
}

gcString BranchInfo::encodeCDKey()
{
	if (m_szCDKey.size() == 0)
		return "";

#ifdef WIN32
	std::string reg = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\MachineGuid", true);
	gcString key("{0}_{1}", reg, m_ItemId.toInt64());

	DATA_BLOB secret;
	secret.pbData = (BYTE*)key.c_str();
	secret.cbData = key.size();

	DATA_BLOB db;
	db.pbData = (BYTE*)m_szCDKey.c_str();
	db.cbData = m_szCDKey.size();


	DATA_BLOB out;

	if (!CryptProtectData(&db, NULL, &secret, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &out))
		return "";

	return UTIL::STRING::base64_encode((char*)out.pbData, out.cbData);
#else // TODO
	return m_szCDKey;
#endif
}

void BranchInfo::decodeCDKey(gcString cdkey)
{
	m_szCDKey = "";

	if (cdkey.size() == 0)
		return;
	
#ifdef WIN32
	size_t outLen = 0;
	char* raw = (char*)UTIL::STRING::base64_decode(cdkey, outLen);

	std::string reg = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\MachineGuid", true);
	gcString key("{0}_{1}", reg, m_ItemId.toInt64());

	DATA_BLOB db;

	db.pbData = (BYTE*)raw;
	db.cbData = outLen;

	DATA_BLOB secret;
	secret.pbData = (BYTE*)key.c_str();
	secret.cbData = key.size();

	DATA_BLOB out;

	if (CryptUnprotectData(&db, NULL, &secret, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &out))
		m_szCDKey.assign((char*)out.pbData, out.cbData);

	safe_delete(raw);
#else // TODO
	m_szCDKey = cdkey;
#endif
}

void BranchInfo::setCDKey(gcString key)
{
	m_szCDKey = key;
	onBranchCDKeyChangedEvent();
}

bool BranchInfo::isCDKeyValid()
{
	return m_szCDKey.size() > 0;
}

DesuraId BranchInfo::getItemId()
{
	return m_ItemId;
}

void BranchInfo::addJSTool(DesuraId toolId)
{
	for (size_t x=0; x<m_vToolList.size(); x++)
	{
		if (m_vToolList[x] == toolId)
			return;
	}

	m_vToolList.push_back(toolId);
}

void BranchInfo::setLinkInfo(const char* name)
{
	m_szName = name;
	m_uiFlags = BF_FREE;

#ifdef WIN32
	m_uiFlags |= BF_WINDOWS_32|BF_WINDOWS_64;
#else
	m_uiFlags |= BF_LINUX_32|BF_LINUX_64;
#endif
}

BranchInstallInfo* BranchInfo::getInstallInfo()
{
	return m_InstallInfo;
}

}
}
