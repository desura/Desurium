/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "VSDownloadMissing.h"

#include "ItemHandle.h"
#include "ItemInfo.h"
#include "usercore/UserCoreI.h"
#include "webcore/WebCoreI.h"

#include "mcfcore/UserCookies.h"
#include "User.h"

namespace UserCore
{
namespace ItemTask
{

VSDownloadMissing::VSDownloadMissing()
{
	m_uiDoneSize = 0;
}

VSDownloadMissing::~VSDownloadMissing()
{
	m_hMcf->getNewProvider() -= delegate(this, &VSDownloadMissing::onNewProvider);

	UserCore::User* pUser = dynamic_cast<UserCore::User*>(getUserCore());

	if (pUser)
		pUser->getBDManager()->cancelDownloadBannerHooks(this);
}


bool VSDownloadMissing::doTask()
{
	m_hMcf->getNewProvider() += delegate(this, &VSDownloadMissing::onNewProvider);

	try
	{
		MCFCore::Misc::UserCookies uc;
		getWebCore()->setMCFCookies(&uc); 

		m_hMcf->getDownloadProviders(getWebCore()->getMCFDownloadUrl(), &uc);
		m_hMcf->dlFilesFromWeb();
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return false;
	}

	if (isStopped())
		return false;

	return checkComplex();
}

void VSDownloadMissing::onProgress(MCFCore::Misc::ProgressInfo& prog)
{
	//normally mcf downloads will count whats all ready done as progress but for verfy we dont want that.
	if (m_uiDoneSize == 0 && prog.doneAmmount > 0)
		m_uiDoneSize = prog.doneAmmount;

	if (prog.totalAmmount > m_uiDoneSize && prog.doneAmmount > m_uiDoneSize)
	{
		prog.totalAmmount -= m_uiDoneSize;
		prog.doneAmmount -= m_uiDoneSize;
	}
}

void VSDownloadMissing::onStop()
{
	m_hMcf->stop();
}

bool VSDownloadMissing::checkComplex()
{
	if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
	{
		UserCore::Item::ItemInfoI* par = getParentItemInfo();

		if (par && (!par->getInstalledModId().isOk() || par->getInstalledModId() != getItemId()))
		{		
			setResult(RES_COMPLETE);
		}
		else if (getItemInfo()->getInstalledModId().isOk())
		{
			setResult(RES_COMPLETE);
		}

		return false;
	}

	return true;
}

void VSDownloadMissing::onNewProvider(MCFCore::Misc::DP_s& dp)
{
	if (isStopped())
		return;

	if (!dp.provider)
		return;

	if (dp.action == MCFCore::Misc::DownloadProvider::ADD)
	{
		UserCore::User* pUser = dynamic_cast<UserCore::User*>(getUserCore());

		if (pUser)
			pUser->getBDManager()->downloadBanner(this, dp.provider);
	}
	else if (dp.action == MCFCore::Misc::DownloadProvider::REMOVE)
	{
		UserCore::Misc::GuiDownloadProvider gdp(MCFCore::Misc::DownloadProvider::REMOVE, dp.provider);
		onNewProviderEvent(gdp);
	}
}

void VSDownloadMissing::onBannerComplete(MCFCore::Misc::DownloadProvider &info)
{
	if (isStopped())
		return;

	UserCore::Misc::GuiDownloadProvider gdp(MCFCore::Misc::DownloadProvider::ADD, &info);
	onNewProviderEvent(gdp);
}

}
}
