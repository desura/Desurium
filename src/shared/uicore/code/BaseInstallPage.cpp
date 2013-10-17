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
#include "MainApp.h"
#include "BaseInstallPage.h"
#include "usercore/ItemHandleI.h"

#include "ItemForm.h"
#include "usercore/ItemManagerI.h"
#include "Managers.h"
#include "managers/WildcardDelegate.h"

class ItemHandleHelper : public UserCore::Item::Helper::ItemHandleHelperI
{
public:
	ItemHandleHelper()
	{
		m_uiId = 0;
	}

	virtual void onChangeStage(uint32 stage)
	{
		onChangeStageEvent(stage);
	}

	virtual void onComplete(uint32 status)
	{
		onCompleteEvent(status);
	}

	virtual void onComplete(gcString& string)
	{
		onCompleteStrEvent(string);
	}

	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info)
	{
		onMcfProgressEvent(info);
	}

	virtual void onProgressUpdate(uint32 progress)
	{
		onProgressUpdateEvent(progress);
	}

	virtual void onError(gcException e)
	{
		onErrorEvent(e);
	}

	virtual void onNeedWildCard(WCSpecialInfo& info)
	{
		onNeedWildCardEvent(info);
	}

	
	virtual void onNewItem(gcString& string)
	{
		onNewItemEvent(string);
	}

	virtual void onItemFound(DesuraId id)
	{
		onItemFoundEvent(id);
	}

	virtual void onDownloadProvider(UserCore::Misc::GuiDownloadProvider& provider)
	{
		onDownloadProviderEvent(provider);
	}

	void onVerifyComplete(UserCore::Misc::VerifyComplete& info)
	{
		onVerifyCompleteEvent(info);
	}

	void onPause(bool state)
	{
		onPauseEvent(state);
	}

	virtual uint32 getId()
	{
		return m_uiId;
	}

	virtual void setId(uint32 id)
	{
		m_uiId = id;
	}


	Event<uint32> onChangeStageEvent;

	Event<uint32> onCompleteEvent;
	Event<gcString> onCompleteStrEvent;

	Event<MCFCore::Misc::ProgressInfo> onMcfProgressEvent;
	Event<uint32> onProgressUpdateEvent;

	Event<gcException> onErrorEvent;
	Event<WCSpecialInfo> onNeedWildCardEvent;

	Event<gcString> onNewItemEvent;
	Event<DesuraId> onItemFoundEvent;

	Event<UserCore::Misc::GuiDownloadProvider> onDownloadProviderEvent;
	Event<UserCore::Misc::VerifyComplete> onVerifyCompleteEvent;

	Event<bool> onPauseEvent;

private:
	uint32 m_uiId;
};

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

typedef void (BaseInstallPage::*onCompleteIntFn)(uint32&);
typedef void (BaseInstallPage::*onCompleteStrFn)(gcString&);


 BaseInstallPage::BaseInstallPage(wxWindow* parent) : BasePage(parent)
 {
	 m_pItemHandle = NULL;
	 m_pIHH = NULL;
 }

 BaseInstallPage::~BaseInstallPage()
 {
	 deregisterHandle();
 }

 
void BaseInstallPage::setInfo(DesuraId id)
{
	BasePage::setInfo(id);
	m_pItemHandle = GetUserCore()->getItemManager()->findItemHandle(id);

	init();
	registerHandle();
}

void BaseInstallPage::pause(bool state)
{
	if (m_pItemHandle)
		m_pItemHandle->setPaused(state);
}

void BaseInstallPage::nonBlockStop()
{
}

void BaseInstallPage::stop()
{
}

void BaseInstallPage::registerHandle()
{
	if (!m_pItemHandle)
		return;

	ItemForm* inf = dynamic_cast<ItemForm*>(GetParent());
	deregisterHandle();

	m_pIHH = new ItemHandleHelper();

	m_pIHH->onCompleteEvent += guiDelegate(this, (onCompleteIntFn)&BaseInstallPage::onComplete);
	m_pIHH->onProgressUpdateEvent += guiDelegate(this, &BaseInstallPage::onProgressUpdate);
	m_pIHH->onErrorEvent += guiDelegate(this, &BaseInstallPage::onError);
	m_pIHH->onNeedWildCardEvent += wcDelegate(inf);

	m_pIHH->onMcfProgressEvent += guiDelegate(this, &BaseInstallPage::onMcfProgress);
	m_pIHH->onCompleteStrEvent += guiDelegate(this, (onCompleteStrFn)&BaseInstallPage::onComplete);
	m_pIHH->onNewItemEvent += guiDelegate(this, &BaseInstallPage::onNewItem);
	m_pIHH->onItemFoundEvent += guiDelegate(this, &BaseInstallPage::onItemFound);
	m_pIHH->onDownloadProviderEvent += guiDelegate(this, &BaseInstallPage::onDownloadProvider);

	m_pIHH->onVerifyCompleteEvent += guiDelegate(this, &BaseInstallPage::onVerifyComplete);
	m_pIHH->onPauseEvent += guiDelegate(this, &BaseInstallPage::onPause);

	m_pItemHandle->addHelper(m_pIHH);
	*m_pItemHandle->getItemInfo()->getInfoChangeEvent() += guiDelegate(this, &BaseInstallPage::onItemUpdate);
}

void BaseInstallPage::deregisterHandle()
{
	if (!m_pItemHandle || !m_pIHH)
		return;

	m_pItemHandle->delHelper(m_pIHH);
	safe_delete(m_pIHH);

	*m_pItemHandle->getItemInfo()->getInfoChangeEvent() -= guiDelegate(this, &BaseInstallPage::onItemUpdate);
}


void BaseInstallPage::onComplete(uint32& status)
{
}

void BaseInstallPage::onProgressUpdate(uint32& progress)
{
}

void BaseInstallPage::onError(gcException& e)
{
}


void BaseInstallPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
}

void BaseInstallPage::onComplete(gcString& str)
{
}

void BaseInstallPage::onNewItem(gcString& item)
{
}

void BaseInstallPage::onItemFound(DesuraId& id)
{
}

void BaseInstallPage::onDownloadProvider(UserCore::Misc::GuiDownloadProvider& provider)
{
}

void BaseInstallPage::onItemUpdate(UserCore::Item::ItemInfoI::ItemInfo_s& info)
{
}

void BaseInstallPage::onVerifyComplete(UserCore::Misc::VerifyComplete& info)
{
}

void BaseInstallPage::onPause(bool &state)
{
}

}
}
}
