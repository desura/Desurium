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

#ifndef DESURA_ITEM_FORM_H
#define DESURA_ITEM_FORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "usercore/ItemHelpersI.h"
#include "usercore/ItemInfoI.h"
#include "usercore/ItemHandleI.h"

class GatherInfoThread;

typedef std::pair<UserCore::Item::Helper::ACTION, gcString> SIPArg;

namespace UI
{
namespace Forms
{

enum INSTALL_ACTION
{
	IA_NONE,
	IA_INSTALL,
	IA_INSTALL_CHECK,
	IA_INSTALL_TESTMCF,
	IA_LAUNCH,
	IA_STARTUP_CHECK,
	IA_SWITCH_BRANCH,
	IA_UNINSTALL,
	IA_UPDATE,
	IA_VERIFY,
	IA_CLEANCOMPLEX,
};

namespace ItemFormPage
{
	class BaseInstallPage;
}

///////////////////////////////////////////////////////////////////////////////
/// Class ItemForm
///////////////////////////////////////////////////////////////////////////////
class ItemForm : public gcFrame, 
	public UserCore::Item::Helper::ItemUninstallHelperI, 
	public UserCore::Item::Helper::ItemLaunchHelperI,
	public UserCore::Item::Helper::ItemHandleFactoryI
{
public:
	ItemForm(wxWindow* parent, const char* action = NULL, const char* id = NULL);
	~ItemForm();

	void finishUninstall(bool complete, bool account);
	void finishInstallCheck();

	void setItemId(DesuraId id);

	void init(INSTALL_ACTION action, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild(), bool showForm = true);
	void newAction(INSTALL_ACTION action, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild(), bool showForm = true);
	void setPaused(bool state = true);

	DesuraId getItemId();

	bool isStopped();

	bool startVerify(bool files, bool tools, bool hooks);
	bool startUninstall(bool complete, bool removeFromAccount);
	bool verifyAfterHashFail();

	void onError(gcException &e);

	bool isInit();

protected:

	Event<bool> onVerifyAfterHashFailEvent;
	void verifyAfterHashFail(bool& res);

	ItemFormPage::BaseInstallPage* m_pPage;	
	wxBoxSizer* m_bsSizer;

	void uninstall();

	void setTitle(const wchar_t* key);
	void onStageChange(uint32 &stage);

	void onFormClose(wxCloseEvent& event);
	void onModalClose(wxCloseEvent& event);
	
	void cleanUpPages();

	void onItemInfoGathered();

	bool verifyItem();
	bool launchItem();
	bool installTestMcf(MCFBranch branch, MCFBuild build);

	virtual void showUpdatePrompt();
	virtual void showLaunchPrompt();
	virtual void showComplexLaunchPrompt();
	virtual void showEULAPrompt();
	virtual void showPreOrderPrompt();
	virtual void launchError(gcException& e);
	virtual bool stopStagePrompt();
	
#ifdef NIX
	virtual void showWinLaunchDialog();
#endif

	virtual void getGatherInfoHelper(UserCore::Item::Helper::GatherInfoHandlerHelperI** helper);
	virtual void getInstallHelper(UserCore::Item::Helper::InstallerHandleHelperI** helper);

	void onShowPlatformError(bool& res);
	void onSelectBranch(std::pair<bool, MCFBranch> &info);
	void onShowComplexPrompt(bool &shouldContinue);
	void onShowInstallPrompt(SIPArg &args);
	void onGatherInfoComplete();
	void onShowError(std::pair<bool, uint8> &args);
	
#ifdef NIX
	void onShowWinLaunchDialog();
	void onShowToolPrompt(std::pair<bool, uint32> &args);
#endif

	EventV onDeleteEvent;
#ifdef NIX
	EventV onShowWinLaunchDialogEvent;
#endif
	void onGatherInfoHandlerHelperDelete(void* &gihh);

	void cleanUpCallbacks();

private:
	GatherInfoThread* m_pGIThread;

	INSTALL_ACTION m_iaLastAction;
	gcWString m_szName;
	gcString m_szLaunchExe;

	DesuraId m_ItemId;
	UserCore::Item::ItemHandleI* m_pItemHandle;

	MCFBuild m_uiMCFBuild; //this holds the build number if we are processing an un authed item
	MCFBranch m_uiMCFBranch;

	bool m_bIsInit;
	
	wxDialog* m_pDialog;
};

}
}

#endif //DESURA_ItemForm_H
