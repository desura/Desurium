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
#include "ItemInfo.h"

#include "XMLMacros.h"
#include "managers/WildcardManager.h"


#include "UserTasks.h"
#include "User.h"

#include "sqlite3x.hpp"
#include "sql/ItemInfoSql.h"

#include "McfManager.h"

#include "BranchInfo.h"
#include "BranchInstallInfo.h"

namespace UM = UserCore::Item;



namespace UserCore
{
namespace Item
{


ItemInfo::ItemInfo(UserCore::User *user, DesuraId id)
{
	initVars();
	m_pUserCore = user;
	m_iId = id;
}


ItemInfo::ItemInfo(UserCore::User *user, DesuraId id, DesuraId parid)
{
	initVars();
	m_pUserCore = user;
	m_iParentId = parid;
	m_iId = id;
}

void ItemInfo::initVars()
{
	m_bWasOnAccount = false;
	m_iPercent = 0;
	m_iStatus = UM::ItemInfoI::STATUS_UNKNOWN;
	m_iParentId = 0;
	m_iPermissions = 0;
	m_iOptions = UM::ItemInfoI::OPTION_AUTOUPDATE;
	m_iChangedFlags = 0;

	m_bPauseCallBack = false;
	m_INBranchIndex = -1;
}

ItemInfo::~ItemInfo()
{
	safe_delete(m_vBranchList);
}


void ItemInfo::deleteFromDb(sqlite3x::sqlite3_connection* db)
{
	try
	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE FROM iteminfo WHERE internalid=?;");
		cmd.bind(1, (long long int)m_iId.toInt64());
		cmd.executenonquery();
	}
	catch (...)
	{
	}

	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		m_vBranchList[x]->deleteFromDb(db);
	}

	std::for_each(m_mBranchInstallInfo.begin(), m_mBranchInstallInfo.end(), [&db](std::pair<uint32, BranchInstallInfo*> p)
	{
		p.second->deleteFromDb(db);
	});

	try
	{
		sqlite3x::sqlite3_command cmd(*db, "DELETE FROM exe WHERE itemid=?;");
		cmd.bind(1, (long long int)m_iId.toInt64());
		cmd.executenonquery();
	}
	catch (...)
	{
	}
}


bool ItemInfo::shouldSaveDb(sqlite3x::sqlite3_connection* db)
{
	if (!db)
		return false;

	bool isDeleted = HasAllFlags(getStatus(), UM::ItemInfoI::STATUS_DELETED);
	bool isOnAccount = HasAllFlags(getStatus(), UM::ItemInfoI::STATUS_ONACCOUNT);
	bool isOnComp = HasAnyFlags(getStatus(), UM::ItemInfoI::STATUS_ONCOMPUTER|UM::ItemInfoI::STATUS_INSTALLED);

	if (isDeleted || (isOnAccount && !isOnComp))
	{
		deleteFromDb(db);
		return false;
	}

	return true;
}

void ItemInfo::saveDb(sqlite3x::sqlite3_connection* db)
{
	if (!shouldSaveDb(db))
		return;

	if (!db)
		return;

	sqlite3x::sqlite3_command cmd(*db, "SELECT count(*) FROM iteminfo WHERE internalid=?;");
	cmd.bind(1, (long long int)m_iId.toInt64());

	int count = cmd.executeint();
		
	if (count == 0)
	{
		saveDbFull(db);
	}
	else
	{
		sqlite3x::sqlite3_command cmd(*db, "UPDATE iteminfo SET "
											"statusflags=?,"
											"percent=?,"
											"icon=?,"
											"logo=?,"
											"iconurl=?,"
											"logourl=?,"
											"ibranch=?,"
											"lastbranch=? WHERE internalid=?;");
		
		uint32 status = m_iStatus&(~UM::ItemInfoI::STATUS_DEVELOPER);

		cmd.bind(1, (int)status); //status
		cmd.bind(2, (int)m_iPercent); //percent
		
		cmd.bind(3, UTIL::OS::getRelativePath(m_szIcon)); //icon
		cmd.bind(4, UTIL::OS::getRelativePath(m_szLogo)); //logo
		cmd.bind(5, m_szIconUrl); //icon
		cmd.bind(6, m_szLogoUrl); //logo

		cmd.bind(7, (int)m_INBranch);
		cmd.bind(8, (int)m_LastBranch);

		cmd.bind(9, (long long int)m_iId.toInt64()); //internal id
		cmd.executenonquery();

		for (size_t x=0; x<m_vBranchList.size(); x++)
		{
			m_vBranchList[x]->saveDb(db);
		}

		std::for_each(m_mBranchInstallInfo.begin(), m_mBranchInstallInfo.end(), [&db](std::pair<uint32, BranchInstallInfo*> p)
		{
			p.second->saveDb(db);
		});
	}
}

void ItemInfo::saveDbFull(sqlite3x::sqlite3_connection* db)
{
	if (!shouldSaveDb(db))
		return;

	if (!db)
		return;

	uint32 status = m_iStatus&(~UM::ItemInfoI::STATUS_DEVELOPER);

	sqlite3x::sqlite3_command cmd(*db, "REPLACE INTO iteminfo VALUES (?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?);");

	cmd.bind(1, (long long int)m_iId.toInt64());		// id
	cmd.bind(2, (long long int)m_iParentId.toInt64());	// parent id
	cmd.bind(3, (int)m_iPercent);	//
	cmd.bind(4, (int)status);		//
	cmd.bind(5, m_szRating);		//

	cmd.bind(6, m_szDev);			//
	cmd.bind(7, m_szName);			//
	cmd.bind(8, m_szShortName);		//
	cmd.bind(9, m_szProfile);		//
	cmd.bind(10, m_szDevProfile);	//

	cmd.bind(11, UTIL::OS::getRelativePath(m_szIcon));			//
	cmd.bind(12, m_szIconUrl);		//
	cmd.bind(13, UTIL::OS::getRelativePath(m_szLogo));			//
	cmd.bind(14, m_szLogoUrl);		//
	cmd.bind(15, m_szPublisher);

	cmd.bind(16, m_szPublisherProfile);
	cmd.bind(17, (int)m_INBranch);
	cmd.bind(18, (int)m_LastBranch);

	cmd.executenonquery();

	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		m_vBranchList[x]->saveDbFull(db);
	}

	std::for_each(m_mBranchInstallInfo.begin(), m_mBranchInstallInfo.end(), [&db](std::pair<uint32, BranchInstallInfo*> p)
	{
		p.second->saveDb(db);
	});
}

void ItemInfo::loadDb(sqlite3x::sqlite3_connection* db)
{
	if (!db)
		return;

	sqlite3x::sqlite3_command cmd(*db, "SELECT * FROM iteminfo WHERE internalid=?;");
	cmd.bind(1, (long long int)m_iId.toInt64());
	sqlite3x::sqlite3_reader reader = cmd.executereader();
	
	reader.read();

	// reader.getint(0); //internal id
	// reader.getint(1); //parent id

	m_iPercent		= reader.getint(2);				 //percent
	m_iStatus		= reader.getint(3);				 //status flags
	m_szRating		= gcString(reader.getstring(4)); //rating

	m_szDev			= gcString(reader.getstring(5)); //developer
	m_szName		= gcString(reader.getstring(6)); //name
	m_szShortName	= gcString(reader.getstring(7)); //shortname
	m_szProfile		= gcString(reader.getstring(8)); //profile url
	m_szDevProfile	= gcString(reader.getstring(9)); //dev profile

	m_szIcon		= UTIL::OS::getAbsPath(reader.getstring(10)); //icon
	m_szIconUrl		= gcString(reader.getstring(11)); //icon url
	m_szLogo		= UTIL::OS::getAbsPath(reader.getstring(12)); //logo
	m_szLogoUrl		= gcString(reader.getstring(13)); //logo url

	m_szPublisher	= reader.getstring(14);
	m_szPublisherProfile = reader.getstring(15);

	m_INBranch		= MCFBranch::BranchFromInt(reader.getint(16));
	m_LastBranch	= MCFBranch::BranchFromInt(reader.getint(17));


	if (HasAnyFlags(m_iStatus, UserCore::Item::ItemInfoI::STATUS_ONACCOUNT))
	{
		m_bWasOnAccount = true;
		m_iStatus &= ~UserCore::Item::ItemInfoI::STATUS_ONACCOUNT;
	}

	delSFlag(UM::ItemInfoI::STATUS_UPDATEAVAL);

	{
		std::vector<uint32> vIdList;

		sqlite3x::sqlite3_command cmd(*db, "SELECT biid FROM installinfo WHERE itemid=?;");
		cmd.bind(1, (long long int)m_iId.toInt64());
		sqlite3x::sqlite3_reader reader = cmd.executereader();
	
		while (reader.read())
		{
			vIdList.push_back(reader.getint(0));
		}

		for (size_t x=0; x<vIdList.size(); x++)
		{
			BranchInstallInfo* bii = new BranchInstallInfo(MCFBranch::BranchFromInt(vIdList[x]), this);

			try
			{
				bii->loadDb(db);
			}
			catch (std::exception &)
			{
				safe_delete(bii);
				continue;
			}

			m_mBranchInstallInfo[vIdList[x]] = bii;
		}
	}

	{
		std::vector<std::pair<uint32,uint32> > vIdList;

		sqlite3x::sqlite3_command cmd(*db, "SELECT branchid, biid FROM branchinfo WHERE internalid=?;");
		cmd.bind(1, (long long int)m_iId.toInt64());
		sqlite3x::sqlite3_reader reader = cmd.executereader();
	
		while (reader.read())
		{
			vIdList.push_back(std::pair<uint32, uint32>(reader.getint(0), reader.getint(1)));
		}

		for (size_t x=0; x<vIdList.size(); x++)
		{
			auto it = m_mBranchInstallInfo.find(vIdList[x].second);

			if (it == m_mBranchInstallInfo.end())
				m_mBranchInstallInfo[vIdList[x].second] = new BranchInstallInfo(vIdList[x].second, this);

			BranchInfo* bi = new BranchInfo(MCFBranch::BranchFromInt(vIdList[x].first), m_iId, m_mBranchInstallInfo[vIdList[x].second]);
			bi->onBranchInfoChangedEvent += delegate(this, &ItemInfo::onBranchInfoChanged);

			try
			{
				bi->loadDb(db);
			}
			catch (std::exception &)
			{
				safe_delete(bi);
				continue;
			}

			if (vIdList[x].first == m_INBranch)
				m_INBranchIndex = x;

			m_vBranchList.push_back(bi);
		}

		// A TYPE_LINK may or may not have branches stored on disk, but will need one at runtime.
		if(getId().getType() == DesuraId::TYPE_LINK && m_vBranchList.size() == 0) {
#ifdef WIN32
			BranchInfo* bi = new BranchInfo(MCFBranch::BranchFromInt(m_INBranch), m_iId, m_mBranchInstallInfo[100]);
#else
#ifdef NIX64
			BranchInfo* bi = new BranchInfo(MCFBranch::BranchFromInt(m_INBranch), m_iId, m_mBranchInstallInfo[120]);
#else
			BranchInfo* bi = new BranchInfo(MCFBranch::BranchFromInt(m_INBranch), m_iId, m_mBranchInstallInfo[110]);
#endif
#endif
			bi->setLinkInfo(getName());
			m_vBranchList.push_back(bi);
			m_INBranchIndex = 0;
		}
	}

	setIconUrl(m_szIconUrl.c_str());
	setLogoUrl(m_szLogoUrl.c_str());

	bool isInstalling = HasAnyFlags(getStatus(), UM::ItemInfoI::STATUS_INSTALLING|UM::ItemInfoI::STATUS_DOWNLOADING|UM::ItemInfoI::STATUS_VERIFING);

	if (!isInstalling)
	{
		addSFlag(UM::ItemInfoI::STATUS_ONCOMPUTER);

		if (getStatus() & UM::ItemInfoI::STATUS_INSTALLCOMPLEX)
		{
			UserCore::MCFManager *mm = UserCore::GetMCFManager();

			gcString path = mm->getMcfPath(this);

			if (UTIL::FS::isValidFile(UTIL::FS::PathWithFile(path)))
			{
				if (!isDownloadable())
					addSFlag(UM::ItemInfoI::STATUS_INSTALLED);
			}
			else
			{
				delSFlag(UM::ItemInfoI::STATUS_INSTALLED);
			}
		}
		else
		{
			BranchInfo* bi = getCurrentBranchFull();

			if (bi && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(bi->getInstallInfo()->getInstallCheck())) )
			{
				if (!isDownloadable())
					addSFlag(UM::ItemInfoI::STATUS_INSTALLED);
			}
			else
			{
				delSFlag(UM::ItemInfoI::STATUS_INSTALLED);
			}
		}
	}

	if ((getStatus() & UM::ItemInfoI::STATUS_INSTALLED) && (getStatus() & UM::ItemInfoI::STATUS_NONDOWNLOADABLE))
		addSFlag(UM::ItemInfoI::STATUS_READY);

	triggerCallBack();
}

void ItemInfo::loadBranchXmlData(TiXmlElement* branch)
{
	const char* szId = branch->Attribute("id");

	if (!szId)
		return;

	uint32 id = atoi(szId);

	BranchInfo* bi = NULL;
	bool found = false;

	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		if (m_vBranchList[x]->getBranchId() == id)
		{
			if (id == m_INBranch)
				m_INBranchIndex = x;

			bi = m_vBranchList[x];
			found = true;
		}
	}

	if (!bi)
	{
		uint32 platformId = 100;
		XML::GetAtt("platformid", platformId, branch);

		auto it = m_mBranchInstallInfo.find(platformId);

		if (it == m_mBranchInstallInfo.end())
			m_mBranchInstallInfo[platformId] = new BranchInstallInfo(platformId, this);

		bi = new BranchInfo(MCFBranch::BranchFromInt(id), m_iId, m_mBranchInstallInfo[platformId], platformId);
		bi->onBranchInfoChangedEvent += delegate(this, &ItemInfo::onBranchInfoChanged);
	}

	bi->loadXmlData(branch);

	if (!found)
	{
		m_vBranchList.push_back(bi);

		size_t x=m_vBranchList.size()-1;
		if (m_vBranchList[x]->getBranchId() == m_INBranch)
			m_INBranchIndex = x;
	}
}

void ItemInfo::loadXmlData(uint32 platform, TiXmlNode *xmlNode, uint16 statusOveride, WildcardManager* pWildCard, bool reset)
{
	if (!xmlNode)
		throw gcException(ERR_BADXML);

	TiXmlElement* xmlEl = xmlNode->ToElement();

	if (!xmlEl)
		throw gcException(ERR_BADXML);

	pauseCallBack();

	TiXmlNode* statNode = xmlNode->FirstChild("status");
	if (statNode)
	{
		TiXmlElement* statEl = statNode->ToElement();

		if (statEl)
		{
			const char* status = statEl->Attribute("id");
			
			bool isDev = (m_iStatus&UM::ItemInfoI::STATUS_DEVELOPER)?true:false;

			if (status)
				m_iStatus = atoi(status);

			if (isDev)
				m_iStatus |= UM::ItemInfoI::STATUS_DEVELOPER;
		}
	}

	bool installed = HasAllFlags(m_iStatus, UM::ItemInfoI::STATUS_INSTALLED);
	bool verifying = HasAllFlags(m_iStatus, UM::ItemInfoI::STATUS_VERIFING);

	addSFlag(statusOveride);
	delSFlag(UM::ItemInfoI::STATUS_INSTALLED);	//need this otherwise installpath and install check dont get set
	
	processInfo(xmlEl);

	XML::for_each_child("branch", xmlNode->FirstChild("branches"), [this](TiXmlElement* branch)
	{
		loadBranchXmlData(branch);
	});

	//the only time settings should be present if the xml came from the api
	TiXmlNode* setNode = xmlNode->FirstChild("settings");
	if (setNode && !isInstalled() && pWildCard)
		processSettings(platform, setNode, pWildCard, reset);

	gcString installCheckFile;

	BranchInfo* bi = getCurrentBranchFull();

	if (bi)
		installCheckFile = bi->getInstallInfo()->getInstallCheck();

	if (getStatus() & UM::ItemInfoI::STATUS_INSTALLCOMPLEX)
	{
		UserCore::MCFManager *mm = UserCore::GetMCFManager();
		installCheckFile = mm->getMcfPath(this);
	}

	if (UTIL::FS::isValidFile(UTIL::FS::PathWithFile(installCheckFile)) && (installed || !isDownloadable()))
	{
		addSFlag(UM::ItemInfoI::STATUS_INSTALLED);

		if (!verifying)
			addSFlag(UM::ItemInfoI::STATUS_READY);
	}

	//work out best branch
	if (isInstalled() && isDownloadable() && m_INBranch == 0)
	{
		std::vector<uint32> vBranchList;

		for (size_t x=0; x<getBranchCount(); x++)
		{
			UserCore::Item::BranchInfoI* bi = getBranch(x);

			if (bi->getFlags() & (UserCore::Item::BranchInfoI::BF_MEMBERLOCK|UserCore::Item::BranchInfoI::BF_REGIONLOCK) )
				continue;

			if (bi->getFlags()&UserCore::Item::BranchInfoI::BF_NORELEASES)
				continue;

			if (bi->getFlags()&(UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_TEST))
				continue;

			if (!(bi->getFlags()&UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !(bi->getFlags()&UserCore::Item::BranchInfoI::BF_FREE))
				continue;

			vBranchList.push_back(x);
		}

		if (vBranchList.size() == 1)
		{
			m_INBranchIndex = vBranchList[0];
			m_INBranch = m_vBranchList[m_INBranchIndex]->getBranchId();
			m_vBranchList[m_INBranchIndex]->getInstallInfo()->setInstalledMcf(m_vBranchList[m_INBranchIndex]->getLatestBuild());
		}
		else
		{
			addSFlag(UserCore::Item::ItemInfoI::STATUS_LINK);
		}
	}
	else if (isInstalled() && !isDownloadable())
	{
		addSFlag(UserCore::Item::ItemInfoI::STATUS_LINK);
	}

	m_iChangedFlags |= UM::ItemInfoI::CHANGED_INFO;
	broughtCheck();

	try
	{
		gcString szItemDb = getItemInfoDb(getUserCore()->getAppDataPath());
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		saveDbFull(&db);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to save item to db: {0}\n", e.what()));
	}

	resumeCallBack();
	triggerCallBack();
}


void ItemInfo::processInfo(TiXmlNode* xmlEl)
{
	//desura info
	XML::GetChild("name", this, &ItemInfo::setName, xmlEl);
	XML::GetChild("nameid", m_szShortName, xmlEl);
	XML::GetChild("summary", m_szDesc, xmlEl);
	XML::GetChild("url", m_szProfile, xmlEl);
	XML::GetChild("style", m_szGenre, xmlEl);
	XML::GetChild("theme", m_szTheme, xmlEl);
	XML::GetChild("rating", m_szRating, xmlEl);
	XML::GetChild("eula", m_szEULAUrl, xmlEl);

	bool isDev = false;
	if (XML::GetChild("devadmin", isDev, xmlEl) && isDev)
		addSFlag(STATUS_DEVELOPER);

	TiXmlNode * logoNode= xmlEl->FirstChild("boxart");
	if (logoNode && logoNode->ToElement())
	{
		const char *icon= logoNode->ToElement()->GetText();

		if (UTIL::MISC::isWebURL(icon))
			setLogoUrl(icon);
		else
			setLogo(icon);
	}

	TiXmlNode * iconNode= xmlEl->FirstChild("icon");
	if (iconNode && iconNode->ToElement())
	{
		const char *icon= iconNode->ToElement()->GetText();

		if (UTIL::MISC::isWebURL(icon))
			setIconUrl(icon);
		else
			setIcon(icon);
	}

	//add support for install info here;



	bool downloadable = false;
	if (XML::GetChild("downloadable", downloadable, xmlEl))
	{
		if (downloadable)
			delSFlag(UM::ItemInfoI::STATUS_NONDOWNLOADABLE);
		else
			addSFlag(UM::ItemInfoI::STATUS_NONDOWNLOADABLE);
	}

	uint32 dlc = 0;
	if (XML::GetChild("expansion", dlc, xmlEl))
	{
		if (dlc)
			addSFlag(UM::ItemInfoI::STATUS_DLC);
		else
			delSFlag(UM::ItemInfoI::STATUS_DLC);
	}

	TiXmlNode* devNode = xmlEl->FirstChild("developer");
	if (devNode)
	{
		XML::GetChild("name", m_szDev, devNode);
		XML::GetChild("url", m_szDevProfile, devNode);
	}

	TiXmlNode* pubNode = xmlEl->FirstChild("publisher");
	if (pubNode)
	{
		XML::GetChild("name", m_szPublisher, pubNode);
		XML::GetChild("url", m_szPublisherProfile, pubNode);
	}
}

void ItemInfo::processSettings(uint32 platform, TiXmlNode* setNode, WildcardManager* pWildCard, bool reset)
{
	//if (platform == -1)
	//	int a=1;

	bool hasBroughtItem = false;

	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		UserCore::Item::BranchInfoI* branch = m_vBranchList[x];

		if (HasAnyFlags(branch->getFlags(), UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !HasAnyFlags(branch->getFlags(), UserCore::Item::BranchInfoI::BF_NORELEASES))
		{
			hasBroughtItem = true;
			break;
		}
	}

	bool installComplex = false;
	XML::GetChild("installcomplex", installComplex, setNode);

	if (installComplex)
		addSFlag(UM::ItemInfoI::STATUS_INSTALLCOMPLEX);
	
	auto it = m_mBranchInstallInfo.find(platform);

	if (it == m_mBranchInstallInfo.end() && !isDownloadable())
	{
		//this item is not downloadable thus has no branches. Create a install info for it.
		m_mBranchInstallInfo[platform] = new BranchInstallInfo(platform, this);
		it = m_mBranchInstallInfo.find(platform);
	}

	if (it != m_mBranchInstallInfo.end())
	{
		uint32 flags = 0;
		char* cip = NULL;

		getUserCore()->getItemManager()->getCIP(m_iId, &cip);

		ProcessResult pr = it->second->processSettings(setNode, pWildCard, reset, hasBroughtItem, cip);

		safe_delete(cip);

		if (pr.useCip || pr.found)
		{
			flags = UM::ItemInfoI::STATUS_ONCOMPUTER;

			if (!isDownloadable() && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(pr.insCheck)))
				flags |= UM::ItemInfoI::STATUS_INSTALLED;

			if (pr.notFirst)
				flags |= UM::ItemInfoI::STATUS_LINK;

			addSFlag(flags);
		}
	}
	else
	{
		Warning("Failed to find platform install settings!\n");
	}
}

void ItemInfo::setIcon(const char* icon)		
{
	if (!UTIL::FS::isValidFile(m_szIcon))
		m_szIcon = "";

	if (!icon)
		return;

	if (!UTIL::FS::isValidFile(icon))
		return;

	m_szIcon = UTIL::FS::PathWithFile(icon).getFullPath();
	m_iChangedFlags |= UM::ItemInfoI::CHANGED_ICON;
	onInfoChange();
}

void ItemInfo::setLogo(const char* logo)		
{
	if (!UTIL::FS::isValidFile(m_szLogo))
		m_szLogo = "";

	if (!logo)
		return;

	UTIL::FS::Path path = UTIL::FS::PathWithFile(logo);

	if (!UTIL::FS::isValidFile(path))
		return;

	m_szLogo = path.getFullPath();
	m_iChangedFlags |= UM::ItemInfoI::CHANGED_LOGO;
	onInfoChange();
}

void ItemInfo::setIconUrl(const char* url)		
{
	if (!url)
		return;

	bool changed = (m_szIconUrl != url);

	if (changed)
		m_szIconUrl = gcString(url);

	if (m_szIconUrl != "" && (changed || !UTIL::FS::isValidFile(m_szIcon)))
		getUserCore()->downloadImage(this, UserCore::Task::DownloadImgTask::ICON);
}

void ItemInfo::setLogoUrl(const char* url)		
{
	if (!url)
		return;

	bool changed = (m_szLogoUrl != url);

	if (changed)
		m_szLogoUrl = gcString(url);

	if (m_szLogoUrl != "" && (changed || !UTIL::FS::isValidFile(m_szLogo)))
		getUserCore()->downloadImage(this, UserCore::Task::DownloadImgTask::LOGO);
}

void ItemInfo::addToAccount()
{
	if (this->getStatus() & UM::ItemInfoI::STATUS_ONACCOUNT)
		return;

	getUserCore()->changeAccount(getId(), UserCore::Task::ChangeAccountTask::ACCOUNT_ADD);

}

void ItemInfo::removeFromAccount()
{
	getUserCore()->changeAccount(getId(), UserCore::Task::ChangeAccountTask::ACCOUNT_REMOVE);
	delSFlag(UM::ItemInfoI::STATUS_ONACCOUNT);
}

void ItemInfo::onInfoChange()
{
	if (!m_bPauseCallBack)
		triggerCallBack();
}

void ItemInfo::triggerCallBack()
{
	ItemInfo_s i;
	i.id = getId();
	i.changeFlags = m_iChangedFlags;

	if (HasAnyFlags(m_iChangedFlags, CHANGED_STATUS) || !getUserCore()->isDelayLoading())
		onInfoChangeEvent(i);
	
	m_iChangedFlags = 0;
}

void ItemInfo::addSFlag(uint32 flags)
{
	if (m_iStatus == flags)
		return;

	bool shouldTriggerUpdate = (m_iStatus&UM::ItemInfoI::STATUS_DEVELOPER || HasAnyFlags(flags, (UM::ItemInfoI::STATUS_INSTALLED|UM::ItemInfoI::STATUS_ONCOMPUTER|UM::ItemInfoI::STATUS_ONACCOUNT|UM::ItemInfoI::STATUS_PAUSED|UM::ItemInfoI::STATUS_UPDATEAVAL)));

	m_iStatus |= flags;
	m_iChangedFlags |= UM::ItemInfoI::CHANGED_STATUS;

	if (flags & UM::ItemInfoI::STATUS_VERIFING)
		delSFlag(UM::ItemInfoI::STATUS_READY);

	//cant be ready and doing other things
	if (flags & UM::ItemInfoI::STATUS_READY)
		delSFlag(UM::ItemInfoI::STATUS_DOWNLOADING|UM::ItemInfoI::STATUS_INSTALLING|UM::ItemInfoI::STATUS_UPLOADING|UM::ItemInfoI::STATUS_VERIFING);

	if (flags & UM::ItemInfoI::STATUS_DOWNLOADING)
		delSFlag(UM::ItemInfoI::STATUS_INSTALLING|UM::ItemInfoI::STATUS_UPLOADING);

	if (flags & UM::ItemInfoI::STATUS_INSTALLING)
		delSFlag(UM::ItemInfoI::STATUS_DOWNLOADING|UM::ItemInfoI::STATUS_UPLOADING);

	if (flags & UM::ItemInfoI::STATUS_UPLOADING)
		delSFlag(UM::ItemInfoI::STATUS_DOWNLOADING|UM::ItemInfoI::STATUS_INSTALLING);

	if (flags & UM::ItemInfoI::STATUS_VERIFING)
		delSFlag(UM::ItemInfoI::STATUS_READY);

	if (shouldTriggerUpdate)
	{
		uint32 num = 1;
		getUserCore()->getItemsAddedEvent()->operator()(num);
	}
	
	onInfoChange();
}

void ItemInfo::delSFlag(uint32 flags)
{
	bool wasDeleted = isDeleted();

	m_iStatus &= (~flags);

	if (flags & STATUS_PAUSABLE)
		m_iStatus &= (~STATUS_PAUSED);

	if (flags & STATUS_INSTALLED)
		m_iStatus &= (~STATUS_READY);

	onInfoChange();
	DesuraId currentID = getId();

	if (!isDeleted() & wasDeleted)
		m_pUserCore->getItemManager()->getOnNewItemEvent()->operator()(currentID);
}

void ItemInfo::addPFlag(uint8 flags)
{
	m_iPermissions |= flags;
}

void ItemInfo::delPFlag(uint8 flags)
{
	m_iPermissions &= (~flags);
}

void ItemInfo::addOFlag(uint8 flags)
{
	m_iOptions |= flags;

	if (flags & UM::ItemInfoI::OPTION_NOUPDATE)
		delOFlag(UM::ItemInfoI::OPTION_PROMPTUPDATE | UM::ItemInfoI::OPTION_AUTOUPDATE);

	if (flags & UM::ItemInfoI::OPTION_PROMPTUPDATE)
		delOFlag(UM::ItemInfoI::OPTION_NOUPDATE | UM::ItemInfoI::OPTION_AUTOUPDATE);

	if (flags & UM::ItemInfoI::OPTION_AUTOUPDATE)
		delOFlag(UM::ItemInfoI::OPTION_PROMPTUPDATE | UM::ItemInfoI::OPTION_NOUPDATE);
}

void ItemInfo::delOFlag(uint8 flags)
{
	m_iOptions &= (~flags);
}

void ItemInfo::setPercent(uint8 percent)
{
	if (m_iPercent == percent)
		return;

#ifdef WIN32
	m_iPercent = min(max(percent,0),100);
#endif
#ifdef NIX
	m_iPercent = std::min(std::max((int)percent,0),100);
#endif

	m_iChangedFlags |= UM::ItemInfoI::CHANGED_PERCENT;

	onInfoChange();
}

bool ItemInfo::isFavorite()
{
	return m_pUserCore->getItemManager()->isItemFavorite(getId());
}

void ItemInfo::setFavorite(bool fav)
{
	m_pUserCore->getItemManager()->setFavorite(getId(), fav);
}

void ItemInfo::updated()
{
	if (getCurrentBranchFull())
		getCurrentBranchFull()->getInstallInfo()->updated();

	delSFlag(UM::ItemInfoI::STATUS_UPDATEAVAL);
}

bool ItemInfo::compare(const char* filter)
{
	gcString f(filter);
	gcString dev(m_szDev);

	std::transform(f.begin(), f.end(), f.begin(), tolower);
	std::transform(dev.begin(), dev.end(), dev.begin(), tolower);

	if (strstr(dev.c_str(), f.c_str())!=NULL)
		return true;


	gcString name(m_szName);
	std::transform(name.begin(), name.end(), name.begin(), tolower);

	if (strstr(name.c_str(), f.c_str())!=NULL)
		return true;


	gcString sname(m_szShortName);
	std::transform(sname.begin(), sname.end(), sname.begin(), tolower);

	if (strstr(sname.c_str(), f.c_str())!=NULL)
		return true;


	gcString theme(m_szTheme);
	std::transform(theme.begin(), theme.end(), theme.begin(), tolower);
	if (strstr(theme.c_str(), f.c_str())!=NULL)
		return true;

	gcString genre(m_szGenre);
	std::transform(genre.begin(), genre.end(), genre.begin(), tolower);


	if (strstr(genre.c_str(), f.c_str())!=NULL)
		return true;

	return false;	
}


void ItemInfo::processUpdateXml(TiXmlNode *node)
{
	TiXmlNode* branches = node->FirstChild("branches");

	if (!branches)
		return;

	TiXmlElement* branch = branches->FirstChildElement("branch");
	while (branch)
	{
		uint32 id;

		const char* szId = branch->Attribute("id");

		if (!szId)
		{
			branch = branch->NextSiblingElement("branch");
			continue;
		}

		id = atoi(szId);

		BranchInfo* bi = NULL;

		for (size_t x=0; x<m_vBranchList.size(); x++)
		{
			if (m_vBranchList[x]->getBranchId() == id)
			{
				bi = m_vBranchList[x];
			}
		}

		if (!bi)
		{
			uint32 platformId = 100;
			XML::GetAtt("platformid", platformId, branch);

			auto it = m_mBranchInstallInfo.find(platformId);

			if (it == m_mBranchInstallInfo.end())
				m_mBranchInstallInfo[platformId] = new BranchInstallInfo(platformId, this);

			bi = new BranchInfo(MCFBranch::BranchFromInt(id), m_iId, m_mBranchInstallInfo[platformId], platformId);
			bi->loadXmlData(branch);

			bi->onBranchInfoChangedEvent += delegate(this, &ItemInfo::onBranchInfoChanged);
			m_vBranchList.push_back(bi);
		}
		else
		{
			bi->loadXmlData(branch);
		}

		if (bi->getBranchId() == m_INBranch)
		{
			if (bi->getInstallInfo()->processUpdateXml(branch))
				addSFlag(UM::ItemInfoI::STATUS_UPDATEAVAL);
		}

		branch = branch->NextSiblingElement("branch");
	}

	broughtCheck();
}

void ItemInfo::broughtCheck()
{
	if (HasAnyFlags(getStatus(), UM::ItemInfoI::STATUS_LINK) == false)
		return;

	bool brought = false;

	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		BranchInfo* bi = m_vBranchList[x];

		bool onAccount = HasAnyFlags(bi->getFlags(), UM::BranchInfoI::BF_ONACCOUNT);
		bool isDemo = HasAnyFlags(bi->getFlags(), UM::BranchInfoI::BF_DEMO|UM::BranchInfoI::BF_TEST);
		
		if (onAccount && !isDemo)
		{
			brought = true;
			break;
		}
	}

	if (brought == false)
		return;

	BranchInfo* bi = getCurrentBranchFull();

	//if we have an installed mod, means we are complex and cant be forgoten about
	if (bi && bi->getInstallInfo()->hasInstalledMod())
		return;

	std::vector<UserCore::Item::ItemInfoI*> modList;
	m_pUserCore->getItemManager()->getModList(getId(), modList);

	for (size_t x=0; x<modList.size(); x++)
	{
		ItemHandleI* ih = m_pUserCore->getItemManager()->findItemHandle(modList[x]->getId());

		//If we are doing something with mods, just dont do any thing
		if (ih->isInStage())
			return;
	}

	uint32 delFlags = 
		UM::ItemInfoI::STATUS_LINK|
		UM::ItemInfoI::STATUS_READY|
		UM::ItemInfoI::STATUS_INSTALLED|
		UM::ItemInfoI::STATUS_ONCOMPUTER|
		UM::ItemInfoI::STATUS_UPDATING|
		UM::ItemInfoI::STATUS_DOWNLOADING|
		UM::ItemInfoI::STATUS_INSTALLING|
		UM::ItemInfoI::STATUS_VERIFING|
		UM::ItemInfoI::STATUS_UPDATEAVAL|
		UM::ItemInfoI::STATUS_PAUSED|
		UM::ItemInfoI::STATUS_PAUSABLE;

	for (size_t x=0; x<modList.size(); x++)
	{
		ItemInfo* i = dynamic_cast<ItemInfo*>(modList[x]);

		if (!i)
			continue;

		i->delSFlag(delFlags);
		i->resetInstalledMcf();
	}

	//forget we are installed so they can install the full version
	delSFlag(delFlags|UM::ItemInfoI::STATUS_NONDOWNLOADABLE);
}

void ItemInfo::resetInstalledMcf()
{
	m_LastBranch = MCFBranch();
	m_INBranch = MCFBranch();
	m_INBranchIndex = -1;

	if (getCurrentBranchFull())
		getCurrentBranchFull()->getInstallInfo()->resetInstalledMcf();

	onInfoChange();
}

bool ItemInfo::setInstalledMcf(MCFBranch branch, MCFBuild build)
{
	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		if (m_vBranchList[x]->getBranchId() == branch)
		{
			m_INBranchIndex = x;

			m_LastBranch = m_INBranch;
			m_INBranch = branch;
			
			if (build == 0)
				build = m_vBranchList[x]->getLatestBuild();

			if (m_vBranchList[x]->getInstallInfo()->setInstalledMcf(build))
				delSFlag(UM::ItemInfoI::STATUS_UPDATEAVAL);

			onInfoChange();
			return true;
		}
	}

	return false;
}

void ItemInfo::overideInstalledBuild(MCFBuild build)
{
	if (!getCurrentBranchFull())
		return;

	getCurrentBranchFull()->getInstallInfo()->overideInstalledBuild(build);
}

bool ItemInfo::hasAcceptedEula()
{
	BranchInfoI* branch = getCurrentBranch();

	if (!branch)
		return true;

	return branch->hasAcceptedEula();
}

const char* ItemInfo::getEulaUrl()	
{
	BranchInfoI* branch = getCurrentBranch();

	if (branch)
	{
		const char* beula = branch->getEulaUrl();

		if (beula && gcString(beula).size() > 0)
			return beula;
	}

	return "";
}

void ItemInfo::acceptEula()
{
	if (m_INBranchIndex == UINT_MAX)
		return;

	if (m_vBranchList[m_INBranchIndex])
		m_vBranchList[m_INBranchIndex]->acceptEula();
}

void ItemInfo::onBranchInfoChanged()
{
	m_iChangedFlags |= UM::ItemInfoI::CHANGED_STATUS;
	onInfoChange();
}

void ItemInfo::setParentId(DesuraId id)
{
	if (id == m_iParentId)
		return;

	sqlite3x::sqlite3_connection db(getItemInfoDb(getUserCore()->getAppDataPath()).c_str());

	try
	{
		sqlite3x::sqlite3_command cmd(db, "DELETE FROM iteminfo WHERE internalid=?;");
		cmd.bind(1, (long long int)m_iId.toInt64());
		cmd.executenonquery();

		saveDbFull(&db);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to change {0} parent id: {1}\n", getName(), e.what()));
	}

	m_iParentId = id;
}

void ItemInfo::migrateStandalone(MCFBranch branch, MCFBuild build)
{
	delSFlag(UserCore::Item::ItemInfoI::STATUS_LINK|UserCore::Item::ItemInfoI::STATUS_NONDOWNLOADABLE);
	setInstalledMcf(branch, build);
}

void ItemInfo::setLinkInfo(const char* exe, const char* args)
{
	if (getId().getType() != DesuraId::TYPE_LINK)
		return;

	UTIL::FS::Path path = UTIL::FS::PathWithFile(exe);

#ifdef WIN32
	uint32 platform = 100;
#else
#ifdef NIX64
	uint32 platform = 120;
#else
	uint32 platform = 110;
#endif
#endif

	if (m_mBranchInstallInfo.size() == 0)
		m_mBranchInstallInfo[platform] = new UserCore::Item::BranchInstallInfo(platform, this);

	BranchInstallInfo *bii = m_mBranchInstallInfo[platform];

	if (m_vBranchList.size() == 0)
		m_vBranchList.push_back(new UserCore::Item::BranchInfo(MCFBranch::BranchFromInt(0), getId(), bii));

	UserCore::Item::BranchInfo* bi = m_vBranchList[0];

	bii->setPath(path.getFolderPath().c_str());
	bii->setInsCheck(exe);
	bii->setLinkInfo(exe, args);
	bi->setLinkInfo(getName());

	m_iStatus = STATUS_LINK|STATUS_NONDOWNLOADABLE|STATUS_READY|STATUS_ONCOMPUTER|STATUS_INSTALLED;

#ifdef WIN32
	gcString savePathIco = UTIL::OS::getAppDataPath(gcWString(getId().getFolderPathExtension("icon.ico")).c_str());
	gcString savePathPng = UTIL::OS::getAppDataPath(gcWString(getId().getFolderPathExtension("icon.png")).c_str());

	try
	{
		UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(savePathIco));
		UTIL::FS::FileHandle fh(savePathIco.c_str(), UTIL::FS::FILE_WRITE);

		UTIL::WIN::extractIcon(exe, [&fh](const unsigned char* buff, uint32 size) -> bool
		{
			fh.write((char*)buff, size);
			return true;
		});

		fh.close();

		UTIL::MISC::convertToPng(savePathIco, savePathPng);
		setIcon(savePathPng.c_str());
	}
	catch (...)
	{
	}
#endif
}

MCFBranch ItemInfo::getBestBranch(MCFBranch branch)
{
	BranchInfoI* bi = NULL;

	if (branch.isGlobal())
	{
		std::vector<BranchInfo*> filterList;

		for (size_t x=0; x<m_vBranchList.size(); x++)
		{
			if (m_vBranchList[x]->getGlobalId() == branch)
				filterList.push_back(m_vBranchList[x]);
		}

		return selectBestBranch(filterList);
	}

	bi = getBranchById(branch);

	if (bi)
		return bi->getBranchId();

	return selectBestBranch(m_vBranchList);
}

MCFBranch ItemInfo::selectBestBranch(const std::vector<BranchInfo*> &list)
{
	if (list.size() == 1)
		return list[0]->getBranchId();

	std::vector<BranchInfo*> shortList;

	bool hasWinBranch = false;
	bool hasNixBranch = false;
	bool is64 = UTIL::OS::is64OS();

	auto shortListBranches = [&](bool ignoreDemo)
	{
		hasWinBranch = false;
		hasNixBranch = false;

		for (size_t x=0; x<list.size(); x++)
		{
			UserCore::Item::BranchInfoI* bi = list[x];

			if (!bi)
				continue;

			if (bi->isWindows())
				hasWinBranch = true;

			if (bi->isLinux())
				hasNixBranch = true;

			uint32 flags = bi->getFlags();

			bool noRelease = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_NORELEASES);
			bool isPreorder = bi->isPreOrder();

			if (noRelease && !isPreorder)
				continue;

			bool free = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_FREE);
			bool onAccount = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_ONACCOUNT);
			bool locked = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_MEMBERLOCK|UserCore::Item::BranchInfoI::BF_REGIONLOCK);

			if (!onAccount && (locked || !free))
				continue;
		
			bool isDemo = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_DEMO);
			bool test = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_TEST);
		
			if ((!ignoreDemo && isDemo) || test)
				continue;

			if (!is64 && list[x]->is64Bit())
				continue;

			shortList.push_back(list[x]);
		}
	};

	shortListBranches(true);

	if (shortList.size() == 0)
		shortListBranches(false);

#ifdef NIX
	//Remove all windows branches if we have a linux branch
	if (hasWinBranch && hasNixBranch)
	{
		std::vector<BranchInfo*> t = shortList;
		shortList.clear();

		for (size_t x=0; x<t.size(); x++)
		{
			if (t[x]->isLinux())
				shortList.push_back(t[x]);
		}
	}
#endif

	if (shortList.size() > 1 && is64)
	{
		std::vector<BranchInfo*> t = shortList;
		shortList.clear();

		for (size_t x=0; x<t.size(); x++)
		{
			if (t[x]->is64Bit())
				shortList.push_back(t[x]);
		}
		
		if (shortList.size() == 0) //filtered all :(
			shortList = t;
	}
	
	if (shortList.size() == 1)
		return shortList[0]->getBranchId();

	return MCFBranch::BranchFromInt(0);
}


BranchInfoI* ItemInfo::getBranch(uint32 index)
{
	if ((int32)index >= m_vBranchList.size())
		return NULL;

	return m_vBranchList[index];
}

BranchInfoI* ItemInfo::getCurrentBranch()
{
	if (m_INBranchIndex == UINT_MAX)
		return NULL;

	return m_vBranchList[m_INBranchIndex];
}

BranchInfo* ItemInfo::getCurrentBranchFull()
{
	if (m_INBranchIndex == UINT_MAX)
		return NULL;

	return m_vBranchList[m_INBranchIndex];
}

BranchInfoI* ItemInfo::getBranchById(uint32 id)
{
	for (size_t x=0; x<m_vBranchList.size(); x++)
	{
		if (m_vBranchList[x]->getBranchId() == id)
			return m_vBranchList[x];
	}

	return NULL;
}


DesuraId ItemInfo::getInstalledModId(MCFBranch branch)
{
	if (!getCurrentBranchFull())
		return DesuraId();

	return getCurrentBranchFull()->getInstallInfo()->getInstalledModId();
}


BranchInstallInfo* ItemInfo::getBranchOrCurrent(MCFBranch branch)
{
	if (branch == 0)
		branch = m_INBranch;

	BranchInfo* bi = dynamic_cast<BranchInfo*>(getBranchById(branch));

	if (!bi && !isDownloadable() && m_mBranchInstallInfo.size() > 0)
		return m_mBranchInstallInfo.begin()->second;

	if (!bi)
		return NULL;

	return bi->getInstallInfo();
}

const char* ItemInfo::getPath(MCFBranch branch)		
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return NULL;

	return bi->getPath();
}

const char* ItemInfo::getInsPrimary(MCFBranch branch)		
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return NULL;

	return bi->getInsPrimary();
}

void ItemInfo::setInstalledModId(DesuraId id, MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return;

	bi->setInstalledModId(id);
}

const char* ItemInfo::getInstalledVersion(MCFBranch branch)	
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return NULL;

	return bi->getInstalledVersion();
}

uint64 ItemInfo::getInstallSize(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return 0;

	return bi->getInstallSize();
}

uint64 ItemInfo::getDownloadSize(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return 0;

	return bi->getDownloadSize();
}

MCFBuild ItemInfo::getLastInstalledBuild(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return MCFBuild();

	return bi->getLastInstalledBuild();
}

MCFBuild ItemInfo::getInstalledBuild(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return MCFBuild();

	return bi->getInstalledBuild();
}

MCFBuild ItemInfo::getNextUpdateBuild(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return MCFBuild();

	return bi->getNextUpdateBuild();
}

void ItemInfo::overrideMcfBuild(MCFBuild build, MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return;

	bi->overideMcfBuild(build);
}

uint32 ItemInfo::getExeCount(bool setActive, MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return 0;

	return bi->getExeCount(setActive);
}

void ItemInfo::getExeList(std::vector<UserCore::Item::Misc::ExeInfoI*> &list, MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return;

	bi->getExeList(list);
}

UserCore::Item::Misc::ExeInfoI* ItemInfo::getActiveExe(MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return NULL;

	return bi->getActiveExe();
}

void ItemInfo::setActiveExe(const char* name, MCFBranch branch)
{
	BranchInstallInfo* bi = getBranchOrCurrent(branch);

	if (!bi)
		return;

	bi->setActiveExe(name);
}

}
}
