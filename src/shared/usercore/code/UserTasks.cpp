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
#include "UserTasks.h"

#include "webcore/WebCoreI.h"
#include "ItemInfo.h"
#include "User.h"
#include "sqlite3x.hpp"

#include "mcfcore/DownloadProvider.h"
#include "webcore/DownloadImageInfo.h"

#ifdef NIX
#include "util/UtilOs.h"
#endif

using namespace UserCore::Item;

namespace UserCore
{
namespace Task
{


DeleteThread::DeleteThread(UserCore::User* user, ::Thread::BaseThread *thread) : UserTask(user)
{
	m_pThread = thread;
}

DeleteThread::~DeleteThread()
{
}

void DeleteThread::doTask()
{
	if (this->isStopped())
		return;

	safe_delete(m_pThread);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


DownloadImgTask::DownloadImgTask(UserCore::User* user, UserCore::Item::ItemInfo* itemInfo, uint8 image) : UserTask(user)
{
	m_Image = image;
	m_pItem = itemInfo;
}

void DownloadImgTask::doTask()
{
	if (!m_pItem)
		return;

	gcString path;

	if (m_Image == ICON)
		path = m_pItem->getIconUrl();
	else
		path = m_pItem->getLogoUrl();

	if (path == "")
		return;

	WebCore::Misc::DownloadImageInfo dii(m_pItem->getId(), path);

	try
	{
		getWebCore()->downloadImage(&dii, m_bStopped);

		if (m_Image == ICON)
			m_pItem->setIcon(dii.outPath.c_str());
		else
			m_pItem->setLogo(dii.outPath.c_str());
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to download image {0}: {1}\n", path, e));
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


ChangeAccountTask::ChangeAccountTask(UserCore::User* user, DesuraId id, uint8 action ) : UserTask(user, id)
{
	m_Action = action;
}

void ChangeAccountTask::doTask()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (!pItem || !getWebCore())
		return;

	try
	{
		if (m_Action == ACCOUNT_ADD)
		{
			getWebCore()->updateAccountItem(pItem->getId(), true);
			pItem->addSFlag(ItemInfoI::STATUS_ONACCOUNT);
		}
		else
		{
			getWebCore()->updateAccountItem(pItem->getId(), false);
			pItem->delSFlag(ItemInfoI::STATUS_ONACCOUNT);

			if (!(pItem->getStatus() & (ItemInfoI::STATUS_INSTALLED|
										ItemInfoI::STATUS_ONCOMPUTER|
										ItemInfoI::STATUS_DOWNLOADING|
										ItemInfoI::STATUS_INSTALLING|
										ItemInfoI::STATUS_UPLOADING|
										ItemInfoI::STATUS_VERIFING)))
			{
				getUserCore()->getItemManager()->removeItem(pItem->getId());
			}
		}
	}
	catch (gcException &except)
	{
		if (except.getSecErrId() != 107)
			Warning(gcString("Error in Change Account task: {0}.\n", except));
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

DownloadBannerTask::DownloadBannerTask(UserCore::User* user, MCFCore::Misc::DownloadProvider *dp) : UserTask(user), m_DPInfo(dp)
{
}

void DownloadBannerTask::doTask()
{
	BannerCompleteInfo bci;

	try
	{
		UTIL::FS::Path path(getUserCore()->getAppDataPath(), "", false);

		path += "temp";
		UTIL::FS::recMakeFolder(path);

		getWebCore()->downloadBanner(&m_DPInfo, path.getFullPath().c_str());
		bci.complete = true;
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to download banner: {0}\n", e));
		bci.complete = false;
	}

	bci.task = this;
	bci.info = m_DPInfo;

	onDLCompleteEvent(bci);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


DownloadAvatarTask::DownloadAvatarTask(UserCore::User* user,  const char* url, uint32 userId) : UserTask(user)
{
	m_szUrl = gcString(url);
	m_uiUserId = userId;
}



void DownloadAvatarTask::doTask()
{
	try
	{
		if (strncmp(m_szUrl.c_str(),"http://", 7)==0)
		{
			HttpHandle wc(m_szUrl.c_str());
			wc->getWeb();
			
			if (wc->getDataSize() != 0)
			{
				if (UTIL::MISC::isValidImage((const unsigned char*)wc->getData()) == IMAGE_VOID)
					throw gcException(ERR_INVALIDDATA, gcString("The url [{0}] is not an image format", m_szUrl));
				
				UTIL::FS::Path urlPath(m_szUrl, "", true);
				UTIL::FS::Path path(getUserCore()->getAppDataPath(), "", false);

				path += "users";
				path += gcString("{0}", m_uiUserId);
				path += urlPath.getFile();

				UTIL::FS::recMakeFolder(path);
				UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_WRITE);

				fh.write(wc->getData(), wc->getDataSize());
				fh.close();

				getUserCore()->setAvatarPath(path.getFullPath().c_str());
			}
			else
			{
				throw gcException(ERR_BADRESPONSE);
			}
		}
	}
	catch (gcException &e)
	{
		onErrorEvent(e);
	}
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


GatherInfoTask::GatherInfoTask(UserCore::User* user,  DesuraId id, bool addToAccount) : UserTask(user, id)
{
	m_bAddToAccount = addToAccount;
}

void GatherInfoTask::doTask()
{
	UserCore::User *pUser = dynamic_cast<UserCore::User*>(getUserCore());

	if (!pUser)
		return;

	try
	{
		uint32 flags = UserCore::Item::ItemInfoI::STATUS_DELETED;

		if (m_bAddToAccount)
			flags = UserCore::Item::ItemInfoI::STATUS_ONACCOUNT;

		pUser->getItemManager()->retrieveItemInfo(getItemId(), flags);

		if (m_bAddToAccount)
		{
			//if we just removed this item from account it will be in a hidden deleted status
			UserCore::Item::ItemInfoI* info = pUser->getItemManager()->findItemInfo(getItemId());

			if (info)
				info->delSFlag(UserCore::Item::ItemInfoI::STATUS_DELETED);
		}
	}
	catch(gcException &e)
	{
		Warning(gcString("Failed to gather item info in task: {0}\n", e));
	}
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


CDKeyTask::CDKeyTask(UserCore::User* user, DesuraId id) : UserTask(user, id)
{
}

void CDKeyTask::doTask()
{
	if (!getItemInfo() || !getWebCore())
	{
		CDKeyEventInfo<gcException> cdei;
		cdei.task = this;
		cdei.t = gcException(ERR_NULLHANDLE, "Item info or web core are null");
		cdei.id = getItemId();

		onErrorEvent(cdei);
		return;
	}

	MCFBranch branch = getItemInfo()->getInstalledBranch();

	try
	{
		gcString key = getWebCore()->getCDKey(getItemId(), branch);

		CDKeyEventInfo<gcString> cdei;
		cdei.task = this;
		cdei.t = key;
		cdei.id = getItemId();

		onCompleteEvent(cdei);
	}
	catch (gcException &e)
	{
		CDKeyEventInfo<gcException> cdei;
		cdei.task = this;
		cdei.t = e;
		cdei.id = getItemId();

		onErrorEvent(cdei);
	}
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class AutoDelFile
{
public:
	AutoDelFile(UTIL::FS::Path path)
	{
		m_Path = path;
	}

	~AutoDelFile()
	{
		UTIL::FS::delFile(m_Path);
	}

	UTIL::FS::Path m_Path;
};


MigrateStandaloneTask::MigrateStandaloneTask(UserCore::User* user, const std::vector<UTIL::FS::Path> &fileList) : UserTask(user, DesuraId())
{
	m_vFileList = fileList;
}

void MigrateStandaloneTask::doTask()
{
	WildcardManager wildc = WildcardManager();
	wildc.onNeedSpecialEvent += delegate(getUserCore()->getNeedWildCardEvent());

	for (size_t x=0; x<m_vFileList.size(); x++)
	{
		AutoDelFile adf(m_vFileList[x]);

		TiXmlDocument doc;

		if (!doc.LoadFile(m_vFileList[x].getFullPath().c_str()))
			continue;

		TiXmlElement* root = doc.FirstChildElement("game");

		if (!root)
			continue;

		gcString path;
		gcString id;

		uint32 branch = -1;
		uint32 build = -1;

		XML::GetChild("path", path, root);
		XML::GetChild("id", id, root);

		XML::GetChild("branch", branch, root);
		XML::GetChild("build", build, root);

		if (id == "" || !UTIL::FS::isValidFolder(path))
			continue;

		DesuraId itemId(id.c_str(), "games");

		UserCore::Item::ItemInfoI* info = getUserCore()->getItemManager()->findItemInfo(itemId);

		if (!info || info->isInstalled())
			continue;

		try
		{
			getUserCore()->getCIPManager()->updateItem(itemId, path);
			getUserCore()->getItemManager()->retrieveItemInfo(getItemId(), 0, &wildc);
		}
		catch (...)
		{
			continue;
		}

		info = getUserCore()->getItemManager()->findItemInfo(itemId);
		UserCore::Item::ItemInfo* realInfo = dynamic_cast<UserCore::Item::ItemInfo*>(info);

		if (!realInfo)
			continue;

		realInfo->migrateStandalone(MCFBranch::BranchFromInt(branch), MCFBuild::BuildFromInt(build));
	}
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

RegenLaunchScriptsTask::RegenLaunchScriptsTask(UserCore::User* user) : UserTask(user, DesuraId())
{
}

void RegenLaunchScriptsTask::doTask()
{
	std::vector<UserCore::Item::ItemHandleI*> itemList;
	
	ItemManagerI* im = getUserCore()->getItemManager();
	
	for (size_t x=0; x<im->getCount(); x++)
	{
		itemList.push_back(im->getItemHandle(x));
	}
	
	for (size_t x=0; x<itemList.size(); x++)
	{
		if (!itemList[x] || !itemList[x]->getItemInfo()->isInstalled())
			continue;

#ifdef NIX
		itemList[x]->installLaunchScripts();
#endif
	}
}



}
}
