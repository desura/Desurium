/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>
          (C) Wojciech Zylinski <voitek@boskee.co.uk>

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
#include "ItemForm.h"
#include "usercore/ItemHandleI.h"

#include "MainApp.h"

#include "InstallGIPage.h"
#include "InstallDLPage.h"
#include "InstallDVPage.h"
#include "InstallINPage.h"
#include "InstallVIPage.h"
#include "InstallINCPage.h"
#include "InstallWaitPage.h"

#include "InstallDLToolPage.h"
#include "InstallINToolPage.h"

#include "ICheckFinishPage.h"
#include "ICheckProgressPage.h"

#include "UninstallInfoPage.h"
#include "UninstallProgressPage.h"

#include "InstallVInfoPage.h"

#include "ComplexPrompt.h"
#include "InstallPrompt.h"
#include "InstallBranch.h"

#include "PreloadPage.h"
#include "managers/WildcardDelegate.h"
#include "Managers.h"

#ifdef NIX
	#include "managers/CVar.h"
#endif

class GatherInfoThread
{
public:
	GatherInfoThread(UI::Forms::ItemForm* parent, DesuraId id, MCFBranch branch)
	{
		m_idItemId = id;
		m_pThread = NULL;
		m_pParent = parent;
		m_McfBranch = branch;
	}

	~GatherInfoThread()
	{
		safe_delete(m_pThread);
	}

	void run()
	{
		m_pThread = GetThreadManager()->newGatherInfoThread(m_idItemId, m_McfBranch, MCFBuild());

		*m_pThread->getErrorEvent() += delegate(this, &GatherInfoThread::onError);
		*m_pThread->getCompleteEvent() += delegate(this, &GatherInfoThread::onComplete);
		*m_pThread->getNeedWCEvent() += wcDelegate(m_pParent);

		m_pThread->start();
	}

	EventV onCompleteEvent;
	Event<gcException> onErrorEvent;

protected:
	void onComplete(uint32&)
	{
		onCompleteEvent();
	}

	void onError(gcException& e)
	{
		onErrorEvent(e);
		onCompleteEvent();
	}

private:
	UI::Forms::ItemForm* m_pParent;
	DesuraId m_idItemId;
	UserCore::Thread::MCFThreadI* m_pThread;
	MCFBranch m_McfBranch;
};


class GatherInfoHandlerHelper : public UserCore::Item::Helper::GatherInfoHandlerHelperI
{
public:
	virtual void destroy()
	{
		void* self = this;
		onDeleteEvent(self);

		delete this;
	}

	virtual void gatherInfoComplete()
	{
		onGatherInfoCompleteEvent();
	}

	virtual bool showError(uint8 flags)
	{
		std::pair<bool, uint8> res(true, flags);
		onShowErrorEvent(res);

		return res.first;
	}

	virtual bool selectBranch(MCFBranch& branch)
	{
		std::pair<bool, MCFBranch> res(true, branch);
		onSelectBranchEvent(res);

		branch = res.second;
		return res.first;
	}

	virtual bool showComplexPrompt()
	{
		bool res = false;
		onShowComplexPromptEvent(res);
		return res;
	}

	virtual bool showPlatformError()
	{
		bool res = false;
		onShowPlatformErrorEvent(res);
		return res;
	}
	
#ifdef NIX
	virtual bool showToolPrompt(UserCore::Item::Helper::TOOL tool)
	{
		std::pair<bool, uint32> res(true, tool);
		onShowToolPromptEvent(res);
		
		return res.first;
	}
#endif

	void onParentDelete()
	{
		onDeleteEvent.reset();

		onshowInstallPromptEvent.reset();
		onShowComplexPromptEvent.reset();
		onShowPlatformErrorEvent.reset();

#ifdef NIX
		onShowToolPromptEvent.reset();
#endif

		onSelectBranchEvent.reset();
		onShowErrorEvent.reset();
		onGatherInfoCompleteEvent.reset();
	}

	virtual UserCore::Item::Helper::ACTION showInstallPrompt(const char* path)
	{
		SIPArg args(UserCore::Item::Helper::C_NONE, path);
		onshowInstallPromptEvent(args);
		return args.first;
	}

	Event<void*> onDeleteEvent;

	Event<SIPArg> onshowInstallPromptEvent;
	Event<bool> onShowComplexPromptEvent;
	
	Event<std::pair<bool, uint8> > onShowErrorEvent;
	Event<std::pair<bool, MCFBranch> > onSelectBranchEvent;
	Event<bool> onShowPlatformErrorEvent;

#ifdef NIX
	Event<std::pair<bool, uint32> > onShowToolPromptEvent;
#endif

	EventV onGatherInfoCompleteEvent;
};




/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////




class InstallerHandleHelper : public UserCore::Item::Helper::InstallerHandleHelperI
{
public:
	InstallerHandleHelper(UI::Forms::ItemForm* parent)
	{
		m_pParent = parent;
	}

	virtual bool verifyAfterHashFail()
	{
		if (!m_pParent)
			return false;

		return m_pParent->verifyAfterHashFail();
	}

	virtual void destroy()
	{
		delete this;
	}

private:
	UI::Forms::ItemForm *m_pParent;
};







namespace UI
{
namespace Forms
{

ItemForm::ItemForm(wxWindow* parent, const char* action, const char* id) : gcFrame( parent, wxID_ANY, L"[Install Form]", wxDefaultPosition, wxSize( 370,280 ), wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxSYSTEM_MENU|wxTAB_TRAVERSAL|wxMINIMIZE_BOX )
{
	Bind(wxEVT_CLOSE_WINDOW, &ItemForm::onFormClose, this);

	SetSizeHints(wxDefaultSize, wxDefaultSize);
	m_bsSizer = new wxBoxSizer(wxVERTICAL);

	m_pPage = new ItemFormPage::PreloadPage(this, action, id);
	m_bsSizer->Add(m_pPage, 1, wxEXPAND, 5);

	this->SetSizer(m_bsSizer);
	this->Layout();

	m_pItemHandle = NULL;
	centerOnParent();

	m_szName = id;
	SetTitle(gcWString(L"{0} {1}", Managers::GetString(L"#IF_TITLE"), m_szName));

	m_iaLastAction = IA_NONE;
	m_pGIThread = NULL;

	onVerifyAfterHashFailEvent += guiDelegate(this, &ItemForm::verifyAfterHashFail, MODE_PENDING_WAIT);
	
#ifdef NIX 
	onShowWinLaunchDialogEvent += guiDelegate(this, &ItemForm::onShowWinLaunchDialog, MODE_PENDING_WAIT); 
#endif 
	
	m_bIsInit = false;
	m_pDialog = NULL;
}

ItemForm::~ItemForm()
{
	if (m_pDialog)
		m_pDialog->EndModal(wxCANCEL);	
	
	safe_delete(m_pGIThread);
	cleanUpCallbacks();
}

void ItemForm::cleanUpCallbacks()
{
	onDeleteEvent();
	onDeleteEvent.reset();
	
	//could be deleted by now. Find it again
	if (GetUserCore() && GetUserCore()->getItemManager())
	{
		m_pItemHandle = GetUserCore()->getItemManager()->findItemHandle(m_ItemId);

		if (m_pItemHandle)
		{
//			*m_pItemHandle->getChangeStageEvent() -= guiDelegate(this, &ItemForm::onStageChange, MODE_PENDING_WAIT);
			*m_pItemHandle->getErrorEvent() -= guiDelegate(this, &ItemForm::onError);
			m_pItemHandle->setFactory(NULL);
		}
	}	
}

bool ItemForm::isStopped()
{
	if (m_pItemHandle)
		return m_pItemHandle->isStopped();

	return false;
}

bool ItemForm::startUninstall(bool complete, bool removeFromAccount)
{
	if (!m_pItemHandle)
		return false;

	return m_pItemHandle->uninstall(this, complete, removeFromAccount);
}

void ItemForm::newAction(INSTALL_ACTION action, MCFBranch branch, MCFBuild build, bool showForm)
{
	if (!isInit())
		init(action, branch, build, showForm);

	if (action == IA_UNINSTALL)
	{
		if (m_pItemHandle)
			m_pItemHandle->setPaused(true);

		uninstall();
	}
	else
	{
		Show();
		Raise();
	}
}

void ItemForm::setItemId(DesuraId id)
{
	m_ItemId = id;
}

void ItemForm::init(INSTALL_ACTION action, MCFBranch branch, MCFBuild build, bool showForm)
{
	m_bIsInit = true;

	if (action == IA_NONE)
	{
		Close();
		return;
	}

	m_pItemHandle = GetUserCore()->getItemManager()->findItemHandle(m_ItemId);

	if (m_pItemHandle)
		*m_pItemHandle->getErrorEvent() += guiDelegate(this, &ItemForm::onError);

	UserCore::Item::ItemInfoI* item = NULL;

	if (m_pItemHandle)
		item = m_pItemHandle->getItemInfo();

	m_uiMCFBuild = build;
	m_uiMCFBranch = branch;

	m_iaLastAction = action;

	if (!item)
	{
		if (!g_pMainApp->isOffline())
		{
			if (action == IA_INSTALL_TESTMCF)
				branch = MCFBranch();

			m_pGIThread = new GatherInfoThread(this, m_ItemId, branch);
			m_pGIThread->onCompleteEvent += guiDelegate(this, &ItemForm::onItemInfoGathered);
			m_pGIThread->onErrorEvent += guiDelegate(this, &ItemForm::onError);
			m_pGIThread->run();
		}
		else
		{
			gcErrorBox(g_pMainApp->getMainWindow(), "#MF_ERRTITLE", "#MF_ERROR", gcException(ERR_LAUNCH, Managers::GetString("#MF_OLNOTINSTALLED")));
			Close();
		}
	}
	else
	{
		*m_pItemHandle->getChangeStageEvent() += guiDelegate(this, &ItemForm::onStageChange, MODE_PENDING_WAIT);

		//if we are currently doing something when setFactory is called it will forward the last events on
		if (m_pItemHandle->isInStage())
		{
			uint32 stage = m_pItemHandle->getStage();
			onStageChange(stage);

			m_pItemHandle->setFactory(this);

			Show();
			Raise();
			return;
		}

		m_pItemHandle->setFactory(this);
		bool res = false;

		switch (action)
		{
			case IA_INSTALL_TESTMCF:
				if (installTestMcf(branch, build))
				{
					res = m_pItemHandle->install(branch, build, true);
					Show();
				}
				break;

			case IA_CLEANCOMPLEX:
				res = m_pItemHandle->cleanComplexMods();
				break;

			case IA_STARTUP_CHECK:
				res = m_pItemHandle->startUpCheck();
				break;

			case IA_SWITCH_BRANCH:
				res = m_pItemHandle->switchBranch(branch);
				break;

			case IA_UPDATE:
				setTitle(L"#IF_UPDATE");
				m_pItemHandle->update();
				res = true;
				break;

			case IA_INSTALL:
				if (!item->getCurrentBranch() || branch != item->getCurrentBranch()->getBranchId())
				{
					res = m_pItemHandle->install(this, branch);
					break;
				}

			case IA_VERIFY:
				res = verifyItem();
				break;

			case IA_LAUNCH:
				res = launchItem();
				break;

			case IA_UNINSTALL:
				uninstall();
				res = true;
				break;

			case IA_INSTALL_CHECK:
				res = m_pItemHandle->installCheck();
				break;
				
			case IA_NONE:
				break;
		};
		
		if (res == false)
		{
			Close();
		}
		else if (showForm)
		{
			m_pItemHandle->setPauseOnError(false);

			Show();
			
#ifdef WIN32
			//some reason on gtk this can cause the title bars around the form to disapear
			Raise();
#endif
		}
	}
}


bool ItemForm::installTestMcf(MCFBranch branch, MCFBuild build)
{
	if (!m_pItemHandle)
		return false;

	UserCore::Item::ItemInfoI* item = m_pItemHandle->getItemInfo();

	if (!GetUserCore()->isAdmin() && !HasAllFlags(item->getStatus(), UserCore::Item::ItemInfoI::STATUS_DEVELOPER))
	{
		gcMessageBox(g_pMainApp->getMainWindow(), Managers::GetString(L"#MF_IPERMISSION_ERROR"),Managers::GetString(L"#MF_PERMISSION_ERRTITLE"));
	}
	else if (build==0)
	{
		gcMessageBox(g_pMainApp->getMainWindow(), Managers::GetString(L"#MF_IBADBUILD"), Managers::GetString(L"#MF_ERRTITLE"));
	}
	else if (branch==0)
	{
		gcMessageBox(g_pMainApp->getMainWindow(), Managers::GetString(L"#MF_IBADBRANCH"), Managers::GetString(L"#MF_ERRTITLE"));
	}
	else
	{
		return true;
	}

	return false;
}

bool ItemForm::verifyItem()
{
	cleanUpPages();

	m_pPage =  new ItemFormPage::InstallVerifyInfoPage(this);
	m_pPage->setInfo(m_pItemHandle->getItemInfo()->getId());

	setTitle(L"#IF_VERIFY");

	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 0 );
	Layout();
	Refresh();

	return true;
}

bool ItemForm::startVerify(bool files, bool tools, bool hooks)
{
	return m_pItemHandle->verify(files, tools, hooks);
}

bool ItemForm::launchItem()
{
	bool offLine = g_pMainApp->isOffline();
	bool res = false;

	if (offLine && !m_pItemHandle->getItemInfo()->isLaunchable())
	{
		gcErrorBox(g_pMainApp->getMainWindow(), "#MF_ERRTITLE", "#MF_ERROR", gcException(ERR_LAUNCH, Managers::GetString("#MF_OLNOTINSTALLED")));
	}
	else
	{
		bool ignoreUpdate = HasAnyFlags(m_pItemHandle->getItemInfo()->getOptions(), UserCore::Item::ItemInfoI::OPTION_NOTREMINDUPDATE);
		res = m_pItemHandle->launch(this, offLine, ignoreUpdate);

		//if res is true we need to keep the form open
		if (res == true)
		{
			Show();
			Raise();
		}
		else
		{
			g_pMainApp->showPlay();
		}
	}

	return res;
}










DesuraId ItemForm::getItemId()
{
	if (!m_pItemHandle)
		return m_ItemId;

	return m_pItemHandle->getItemInfo()->getId();
}

void ItemForm::setPaused(bool state)
{
	m_pItemHandle->setPaused(state);
}

void ItemForm::setTitle(const wchar_t* key)
{
	UserCore::Item::ItemInfoI* item = m_pItemHandle?m_pItemHandle->getItemInfo():NULL;
	if (item)
	{
		m_szName = gcWString(item->getName());
		SetTitle(gcWString(L"{0} {1}", Managers::GetString(key), m_szName));
	}
	else
	{
		SetTitle(Managers::GetString(key));
	}
}

void ItemForm::uninstall()
{
	cleanUpPages();

	m_pPage =  new ItemFormPage::UninstallInfoPage(this);
	m_pPage->setInfo(m_pItemHandle->getItemInfo()->getId());

	setTitle(L"#IF_UNINSTALL");

	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 0 );
	Layout();
	Refresh();
}

void ItemForm::finishUninstall(bool complete, bool account)
{
	m_pItemHandle->uninstall(this, complete, account);
}

void ItemForm::finishInstallCheck()
{
	cleanUpPages();

	m_pPage =  new ItemFormPage::ICheckFinishPage(this);
	m_pPage->setInfo(m_pItemHandle->getItemInfo()->getId());

	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 0 );
	Layout();
	Refresh();

	m_pPage->init();
}

void ItemForm::onStageChange(uint32 &stage)
{
	if (stage == UserCore::Item::ItemHandleI::STAGE_CLOSE)
	{
		Close();
		return;
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_LAUNCH)
	{
		Show(false);
		m_pItemHandle->launch(this, g_pMainApp->isOffline());

		return;
	}

	cleanUpPages();

	if (stage == UserCore::Item::ItemHandleI::STAGE_INSTALL)
	{
		setTitle(L"#IF_INSTALL");
		m_pPage = new ItemFormPage::InstallINPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_INSTALL_COMPLEX)
	{
		setTitle(L"#IF_INSTALL");
		m_pPage = new ItemFormPage::InstallINCPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_DOWNLOAD)
	{
		setTitle(L"#IF_DOWNLOAD");
		m_pPage = new ItemFormPage::InstallDLPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_VERIFY)
	{
		setTitle(L"#IF_VERIFY");
		m_pPage = new ItemFormPage::InstallVIPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_GATHERINFO)
	{
		setTitle(L"#IF_TITLE");
		m_pPage = new ItemFormPage::InstallGIPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_UNINSTALL)
	{
		setTitle(L"#IF_UNINSTALL");
		m_pPage = new ItemFormPage::UninstallProgressPage(this, L"#IF_UNINSTALL_LABEL");
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_UNINSTALL_BRANCH)
	{
		setTitle(L"#IF_UNINSTALL_BRANCH");
		m_pPage = new ItemFormPage::UninstallProgressPage(this, L"#IF_UNINSTALL_BRANCH_LABEL");
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_UNINSTALL_PATCH || stage == UserCore::Item::ItemHandleI::STAGE_UNINSTALL_UPDATE)
	{
		setTitle(L"#IF_UNINSTALL_PATCH");
		m_pPage = new ItemFormPage::UninstallProgressPage(this, L"#IF_UNINSTALL_PATCH_LABEL");
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_UNINSTALL_COMPLEX)
	{
		if (m_pItemHandle)
		{
			m_szName = gcWString(m_pItemHandle->getItemInfo()->getName());
			SetTitle(gcWString(L"{0}: {1}", m_szName, Managers::GetString(L"#IF_UNINSTALL_COMPLEX")));
		}
		else
		{
			setTitle(L"#IF_UNINSTALL_COMPLEX");
		}

		m_pPage = new ItemFormPage::UninstallProgressPage(this, L"#IF_UNINSTALL_COMPLEX_LABEL");
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_INSTALL_CHECK)
	{
		setTitle(L"#IF_INSTALL_CHECK");
		m_pPage = new ItemFormPage::ICheckProgressPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_DOWNLOADTOOL)
	{
		setTitle(L"#IF_DOWNLOADTOOL");
		m_pPage = new ItemFormPage::InstallDLToolPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_INSTALLTOOL)
	{
		setTitle(L"#IF_INSTALLTOOL");
		m_pPage = new ItemFormPage::InstallINToolPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_VALIDATE)
	{
		setTitle(L"#IF_VALIDATE_TITLE");
		m_pPage = new ItemFormPage::InstallDVPage(this);
	}
	else if (stage == UserCore::Item::ItemHandleI::STAGE_WAIT)
	{
		setTitle(L"#IF_WAIT_TITLE");
		m_pPage = new ItemFormPage::InstallWaitPage(this);
	}
	else
	{
		//shouldnt get here!!!!!
		return;
	}

	m_pPage->setMCFInfo(m_uiMCFBranch, m_uiMCFBuild);
	m_pPage->setInfo(m_pItemHandle->getItemInfo()->getId());

	m_pPage->Show(true);

	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	Refresh();
}



void ItemForm::onItemInfoGathered()
{
	if (!m_pItemHandle)
	{
		m_pItemHandle = GetUserCore()->getItemManager()->findItemHandle(m_ItemId);

		if (m_pItemHandle)
		{
			*m_pItemHandle->getErrorEvent() += guiDelegate(this, &ItemForm::onError);

			setItemId(m_pItemHandle->getItemInfo()->getId());
			init(m_iaLastAction, m_uiMCFBranch, m_uiMCFBuild);
			Show();
			Raise();
		}
		else
		{
			Warning("Item cannot be found from gather info return.\n");
			Close();
		}
	}

	safe_delete(m_pGIThread);
}

void ItemForm::onFormClose(wxCloseEvent& event)
{
	if (m_pDialog)
		m_pDialog->EndModal(wxCANCEL);
	
	Show(false);
	cleanUpCallbacks();
	
	g_pMainApp->closeForm(this->GetId());
	cleanUpPages();
}

void ItemForm::cleanUpPages()
{
	m_bsSizer->Clear(false);

	if (m_pPage)
	{
		m_pPage->deregisterHandle();

		m_pPage->Show(false);
		m_pPage->Close();
		m_pPage = NULL;
	}
}






void ItemForm::showUpdatePrompt()
{
	g_pMainApp->handleInternalLink(m_ItemId, ACTION_PROMPT, FormatArgs("prompt=update"));
}

void ItemForm::showLaunchPrompt()
{
	g_pMainApp->handleInternalLink(m_ItemId, ACTION_PROMPT, FormatArgs("prompt=launch"));
}

void ItemForm::showComplexLaunchPrompt()
{
	g_pMainApp->handleInternalLink(m_ItemId, ACTION_PROMPT, FormatArgs("prompt=complexlaunch"));
}

void ItemForm::showEULAPrompt()
{
	g_pMainApp->handleInternalLink(m_ItemId, ACTION_PROMPT, FormatArgs("prompt=eula"));
}

void ItemForm::showPreOrderPrompt()
{
	g_pMainApp->handleInternalLink(m_ItemId, ACTION_PROMPT, FormatArgs("prompt=preload"));
}

#ifdef NIX
CVar gc_linux_disable_windows_warning("gc_linux_disable_windows_warning", "false");

void ItemForm::showWinLaunchDialog()
{
	//if (wxThread::IsMain())
	//	onShowWinLaunchDialog();
	//else
		onShowWinLaunchDialogEvent();
}

void ItemForm::onShowWinLaunchDialog() 
{
	if (! gc_linux_disable_windows_warning.getBool())
		gcMessageBox(this, Managers::GetString(L"#IF_WINDOWS_LAUNCH_WARNING")); 
} 
#endif


class LaunchErrorHelper : public HelperButtonsI
{
public:
	LaunchErrorHelper(UserCore::Item::ItemHandleI* itemHandle)
	{
		m_pItemHandle = itemHandle;
	}

	virtual uint32 getCount()
	{
		return 1;
	}

	virtual const wchar_t* getLabel(uint32 i)
	{
		return Managers::GetString(L"#P_VERIFY");
	}

	virtual const wchar_t* getToolTip(uint32 i)
	{
		return Managers::GetString(L"#MF_LUANCH_ERROR_VERIFY_TOOLTIP");
	}

	virtual void performAction(uint32 i)
	{
		if (m_pItemHandle)
			m_pItemHandle->verify(true, true, true);
	}

private:
	UserCore::Item::ItemHandleI* m_pItemHandle;
};



void ItemForm::launchError(gcException& e)
{
	LaunchErrorHelper leh(m_pItemHandle);

#ifdef NIX64
	switch (e.getErrId())
	{
		case ERR_NO32LIBS:
			gcErrorBox(g_pMainApp->getMainWindow(), "#MF_ERRTITLE", "#MF_ERROR_NO32LIBS", e, NULL);
			return;
		case ERR_NOBITTEST:
			gcErrorBox(g_pMainApp->getMainWindow(), "#MF_ERRTITLE", "#MF_ERROR_NOBITTEST", e, NULL);
			return;
	}
#endif

	gcErrorBox(g_pMainApp->getMainWindow(), "#MF_ERRTITLE", "#MF_ERROR", e, &leh);
}

bool ItemForm::stopStagePrompt()
{
	if (!m_pItemHandle)
		return false;

	gcString title(Managers::GetString("#MF_UNINSTALLTITLE"), m_pItemHandle->getItemInfo()->getName());
	gcString prompt(Managers::GetString("#MF_UNINSTALLPROMPT"), m_pItemHandle->getItemInfo()->getName());

	return gcMessageBox(this, prompt, title, wxICON_QUESTION|wxYES_NO) == wxYES;
}

void ItemForm::getGatherInfoHelper(UserCore::Item::Helper::GatherInfoHandlerHelperI** helper)
{
	GatherInfoHandlerHelper *gihh = new GatherInfoHandlerHelper();

	*helper = gihh;

	gihh->onSelectBranchEvent += guiDelegate(this, &ItemForm::onSelectBranch, MODE_PENDING_WAIT);
	gihh->onShowComplexPromptEvent += guiDelegate(this, &ItemForm::onShowComplexPrompt, MODE_PENDING_WAIT);
	gihh->onshowInstallPromptEvent += guiDelegate(this, &ItemForm::onShowInstallPrompt, MODE_PENDING_WAIT);
	gihh->onShowPlatformErrorEvent += guiDelegate(this, &ItemForm::onShowPlatformError, MODE_PENDING_WAIT);

#ifdef NIX
	gihh->onShowToolPromptEvent += guiDelegate(this, &ItemForm::onShowToolPrompt, MODE_PENDING_WAIT);
#endif

	gihh->onGatherInfoCompleteEvent += guiDelegate(this, &ItemForm::onGatherInfoComplete);
	gihh->onShowErrorEvent += guiDelegate(this, &ItemForm::onShowError, MODE_PENDING_WAIT);

	onDeleteEvent += delegate(gihh, &GatherInfoHandlerHelper::onParentDelete);
	gihh->onDeleteEvent += delegate(this, &ItemForm::onGatherInfoHandlerHelperDelete);
}

void ItemForm::onGatherInfoHandlerHelperDelete(void* &gihh)
{
	onDeleteEvent -= delegate((GatherInfoHandlerHelper*)gihh, &GatherInfoHandlerHelper::onParentDelete);
}

void ItemForm::getInstallHelper(UserCore::Item::Helper::InstallerHandleHelperI** helper)
{
	*helper = new InstallerHandleHelper(this);
}







void ItemForm::onModalClose(wxCloseEvent& event)
{
	if (m_pDialog && m_pDialog->GetId() == event.GetId())
		m_pDialog = NULL;
}


void ItemForm::onSelectBranch(std::pair<bool, MCFBranch> &info)
{
	if (m_uiMCFBranch)
	{
		info.second = m_uiMCFBranch;
		return;
	}

	InstallBranch *prompt = new InstallBranch(this);
	prompt->Bind(wxEVT_CLOSE_WINDOW, &ItemForm::onModalClose, this);
	
	m_pDialog = prompt;
	
	int res = prompt->setInfo(m_pItemHandle->getItemInfo()->getId(), true);
		
	if (res == 1 || (prompt->ShowModal() == wxID_OK && m_pDialog))
	{
		info.second = prompt->getGlobal();

		if (info.second == 0)
			info.second = prompt->getBranch();
	}

	if (info.second == 0)
		info.first = false;
		
	if (m_pDialog)
	{
		m_pDialog = NULL;
		prompt->Close();
	}
}

void ItemForm::onShowComplexPrompt(bool &shouldContinue)
{
	ComplexPrompt *prompt = new ComplexPrompt(this);
	prompt->Bind(wxEVT_CLOSE_WINDOW, &ItemForm::onModalClose, this);
	
	m_pDialog = prompt;
	
	prompt->setInfo(m_pItemHandle->getItemInfo()->getId());
	shouldContinue = (prompt->ShowModal() == wxID_OK);
	
	if (m_pDialog)
	{
		m_pDialog = NULL;
		prompt->Close();
	}
}

void ItemForm::onShowInstallPrompt(SIPArg &args)
{
	InstallPrompt *prompt = new InstallPrompt(this);
	prompt->Bind(wxEVT_CLOSE_WINDOW, &ItemForm::onModalClose, this);
	
	m_pDialog = prompt;
	
	prompt->setInfo(m_pItemHandle->getItemInfo()->getId(), args.second.c_str());

	UserCore::Item::Helper::ACTION action = UserCore::Item::Helper::C_NONE;

	if (prompt->ShowModal() == wxID_OK && m_pDialog)
	{
		switch (prompt->getChoice())
		{
		case C_VERIFY:
			action = UserCore::Item::Helper::C_VERIFY;
			break;

		case C_INSTALL:
			action = UserCore::Item::Helper::C_INSTALL;
			break;

		case C_REMOVE:
			action = UserCore::Item::Helper::C_REMOVE;
			break;
		};
	}

	args.first = action;
	
	if (m_pDialog)
	{
		m_pDialog = NULL;
		prompt->Close();
	}
}

void ItemForm::onShowPlatformError(bool& res)
{
	UserCore::Item::ItemInfoI* item = m_pItemHandle->getItemInfo();

	gcString name("Unknown Item ({0}: {0})", getItemId().getTypeString(), getItemId().getItem());

	if (item)
		name = item->getName();

	gcString errMsg(Managers::GetString("#IF_GERROR_BRANCH_PLATFORM"), name);

	errMsg += "\n\n";
	errMsg += Managers::GetString("#IF_GERROR_BRANCH_PLATFORM_Q");

	int answer = gcMessageBox(this, errMsg.c_str(), Managers::GetString("#IF_GERRTITLE"), wxICON_EXCLAMATION|wxYES|wxNO);
	res = (answer == wxYES);

	if (res)
		m_uiMCFBranch = MCFBranch::BranchFromInt(0);
}

#ifdef NIX

class ShowToolPromptHelper : public HelperButtonsI
{
public:
	virtual uint32 getCount()
	{
		return 1;
	}
	
	virtual const wchar_t* getLabel(uint32 index)
	{
		return Managers::GetString(L"#HELP");
	}
	
	virtual const wchar_t* getToolTip(uint32 index)
	{
		return NULL;
	}
	
	virtual void performAction(uint32 index)
	{
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::LinuxToolHelp).c_str(), SUPPORT);
	}
};


void ItemForm::onShowToolPrompt(std::pair<bool, uint32> &args)
{
	UserCore::Item::ItemInfoI* item = m_pItemHandle->getItemInfo();

	gcString name("Unknown Item ({0}: {0})", getItemId().getTypeString(), getItemId().getItem());

	if (item)
		name = item->getName();	


	const char* tool = NULL;
	
	switch (args.second)
	{
		case UserCore::Item::Helper::JAVA_SUN:
			tool = "Java (Sun)";
			break;
			
		case UserCore::Item::Helper::JAVA:
			tool = "Java (Any)";
			break;
			
		case UserCore::Item::Helper::MONO:
			tool = "Mono";
			break;
			
		case UserCore::Item::Helper::AIR:
			tool = "Adobe Air";
			break;
		
		default:
			return;
	}
	
	gcString errMsg(Managers::GetString("#IF_GERROR_TOOL"), tool, name);

	errMsg += "\n\n";
	errMsg += Managers::GetString("#IF_GERROR_TOOL_Q");	

	ShowToolPromptHelper helper;
	int answer = gcMessageBox(this, errMsg.c_str(), Managers::GetString("#IF_GERRTITLE"), wxICON_QUESTION|wxYES|wxNO, &helper);
	args.first = (answer == wxYES);
}
#endif

void ItemForm::onGatherInfoComplete()
{
	if (m_pItemHandle->getItemInfo()->isDownloadable())
	{
		gcWString name(m_pItemHandle->getItemInfo()->getName());
		gcWString msg;

		if (m_pItemHandle->getItemInfo()->isInstalled())
			msg = gcWString(L"{0} {1}", name, Managers::GetString(L"#IC_FOUND"));
		else
			msg = gcWString(L"{0} {1}", name, Managers::GetString(L"#IC_NOTFOUND"));	

		gcMessageBox(this, msg, Managers::GetString(L"#IC_TITLE") );
	}
}

void ItemForm::onShowError(std::pair<bool, uint8> &args)
{
	uint32 flags = args.second;

	if (flags & UserCore::Item::Helper::V_PARENT)
	{
		InstallBranch prompt(this);
		int res = prompt.setInfo(m_pItemHandle->getItemInfo()->getId(), false);
		
		//if ok is returned we should continue the install
		if (res == 1 || prompt.ShowModal() == wxID_OK)
			args.first = false;
	}
	else if (flags & UserCore::Item::Helper::V_BADPATH)
	{
		gcMessageBox(this, Managers::GetString(L"#IF_IIPATH"), Managers::GetString(L"#IF_IIERRTITLE"));
	}
	else if (flags & UserCore::Item::Helper::V_FREESPACE)
	{
		gcMessageBox(this, Managers::GetString(L"#IF_IIFREESPACE"), Managers::GetString(L"#IF_IIERRTITLE"));
	}
	else
	{
		gcMessageBox(this, Managers::GetString(L"#IF_IIUNKNOWN"), Managers::GetString(L"#IF_IIERRTITLE"));
	}
}

bool ItemForm::verifyAfterHashFail()
{
	bool res = false;
	onVerifyAfterHashFailEvent(res);
	return res;
}

void ItemForm::verifyAfterHashFail(bool& res)
{
	int mbRes = gcMessageBox(this, Managers::GetString(L"#IF_IHASHFAIL"), Managers::GetString(L"#IF_IERRTITLE"), wxICON_QUESTION|wxYES_NO);
	res = (mbRes == wxYES);
}

void ItemForm::onError(gcException &e)
{
	gcErrorBox(this, "#IF_GERRTITLE", "#IF_GERROR", e);
}


bool ItemForm::isInit()
{
	return m_bIsInit;
}




}
}



