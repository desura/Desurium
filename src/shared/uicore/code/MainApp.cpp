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
#include "MainApp.h"

#include <wx/wx.h>
#include "TaskBarIcon.h"
#include "wx/window.h"
#include "Managers.h"
#include "Log.h"
#include <branding/uicore_version.h>
#include "mcfcore/MCFMain.h"
#include "mcfcore/UserCookies.h"

#include "managers/CVar.h"

#include "LoginForm.h"
#include "usercore/NewsItem.h"
#include "wx/taskbar.h"
#include "MainForm.h"


#include <wx/uri.h>

#include "InternalLink.h"
#include "DesuraServiceError.h"

#ifdef WITH_GTEST
#include "gcUnitTestPage.h"
#endif

extern void DeleteCookies();
extern void SetCookies();

extern CVar gc_savelogin;
extern CVar admin_developer;
extern CVar gc_updateduninstall;

gcString g_szUICoreVersion("{0}.{1}.{2}.{3}", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);

void InitManagers();
void InitLocalManagers();
void DestroyManagers();
void SetCrashDumpSettings(const wchar_t* user, bool upload);

const char* GetUICoreVersion()
{
	return g_szUICoreVersion.c_str();
}

//this is the handle to webcore and related functions
UserCore::UserI* g_pUserHandle = NULL;

WebCore::WebCoreI* GetWebCore()
{
	if (g_pUserHandle)
		return g_pUserHandle->getWebCore();

	return NULL;
}

UserCore::UserI* GetUserCore()
{
	return g_pUserHandle;
}



extern const char* GetAppVersion();
extern CVar gc_lastusername;
extern CVar gc_uploaddumps;
extern CVar gc_cleanmcf;
extern CVar gc_corecount;
extern CVar gc_silentlaunch;

extern ConCommand cc_PrintVersion;
extern ConCommand cc_CheckCert;


class DeleteUserThread : public ::Thread::BaseThread
{
public:
	DeleteUserThread(UserCore::UserI* user) : ::Thread::BaseThread("Delete User Thread")
	{
		m_pUser = user;
	}
	
	virtual void run()
	{
		safe_delete(m_pUser);	
	}
	
	UserCore::UserI* m_pUser;
};

DeleteUserThread* g_pDeleteThread = NULL;

class AutoDelDeleteThread
{
public:
	~AutoDelDeleteThread()
	{
		safe_delete(g_pDeleteThread);
	}
};

AutoDelDeleteThread addt;

const char* g_szSafeList[] = 
{
	".desura.com",
	"desura.com",
	".paypal.com",
	"paypal.com",
	".google.com",
	"google.com",
	NULL,
};

bool isSafeUrl(const char* url)
{

	if (!url)
		return false;

	if (gcString(url) == "about:blank")
		return true;

	wxURI wxurl(url);

	if (wxurl.GetScheme() == "file")
		return true;

	wxString server = wxurl.GetServer();

	size_t x=0;

	while (g_szSafeList[x])
	{
		if (g_szSafeList[x][0] == '.')
		{
			if (server.Contains(wxString(g_szSafeList[x])))
				return true;
		}
		else
		{
			if (server == wxString(g_szSafeList[x]))
				return true;
		}

		x++;
	}
	
	return false;
}

extern CVar gc_autostart;
extern CVar gc_enable_api_debugging;

wxWindow* GetMainWindow(wxWindow* p)
{
	if (!p || p == g_pMainApp)
		return g_pMainApp->getTopLevelWindow();
	
	return p;
}

MainApp::MainApp()
{
	Bind(wxEVT_CLOSE_WINDOW, &MainApp::onClose, this);

	m_wxLoginForm = NULL;
	m_wxTBIcon = NULL;
	m_wxMainForm = NULL;
	
	m_bQuiteMode = false;
	m_bLoggedIn = false;
	m_iMode = MODE_LOGOUT;

	//need to overide the value in corecount if not set
	if (gc_corecount.getInt() == 0)
	{
		//need to change the value so it will trigger the cvar callback
		gc_corecount.setValue(1);
		gc_corecount.setValue(0);
	}

	m_pOfflineDialog = NULL;
	m_pInternalLink = NULL;
#ifdef WITH_GTEST
	m_UnitTestForm = NULL;
#endif
	onLoginAcceptedEvent += guiDelegate(this, &MainApp::onLoginAcceptedCB);
	onInternalLinkEvent += guiDelegate(this, &MainApp::onInternalLink);
	onInternalLinkStrEvent += guiDelegate(this, &MainApp::onInternalStrLink);
	onNotifyGiftUpdateEvent += guiDelegate(this, &MainApp::onNotifyGiftUpdate);
}

MainApp::~MainApp()
{
	if (m_pOfflineDialog)
		m_pOfflineDialog->EndModal(0);

#ifdef WITH_GTEST
	if (m_UnitTestForm)
	{
		m_UnitTestForm->canClose();
		m_UnitTestForm->Close();
	}
#endif

	safe_delete(m_vNewsItems);
	safe_delete(m_pInternalLink);

	//delete user first so threads will not die when they try to access webcore
	//should be deleted on logout but just to make sure
	Thread::AutoLock a(m_UserLock);
	
	if (m_wxTBIcon)
		m_wxTBIcon->deregEvents();

	safe_delete(g_pDeleteThread);
	g_pDeleteThread = new DeleteUserThread(g_pUserHandle);
	
#ifdef NIX
	g_pDeleteThread->start();
#else
	g_pDeleteThread->run();
	safe_delete(g_pDeleteThread);
#endif

	g_pUserHandle = NULL;
	safe_delete(m_wxTBIcon);

	DestroyManagers();
	DestroyLogging();	
}

gcFrame* MainApp::getMainWindow()
{
	return m_wxMainForm;
}

wxWindow* MainApp::getTopLevelWindow()
{
	if (m_wxLoginForm)
		return m_wxLoginForm;

	return m_wxMainForm;
}

void MainApp::Init(int argc, wxCmdLineArgsArray &argv)
{
	if (argc > 0)
	{
		for (int x=0; x<argc; x++)
		{
			wxString str = argv[x].MakeLower();

			if (str == "-dgl")
				m_bQuiteMode = true;

			if (str.StartsWith("desura://"))
				m_szDesuraCache = gcString(argv[x].ToStdString());
		}
	}

	//char *comAppPath = NULL;
	//UTIL::OS::getAppDataPath(&comAppPath);
	//UTIL::FS::recMakeFolder(comAppPath);
	//safe_delete(comAppPath);

	InitLogging();
	cc_PrintVersion();
	cc_CheckCert();
	Msg("Logging has started\n");
	Msg("\n\n");

	InitManagers();
	InitLocalManagers();

	std::string val = UTIL::OS::getConfigValue(REGRUN);
	gc_autostart.setValue( val.size() > 0 );

	//because logging gets init first we need to man reg it
	RegLogWithWindow();

	//because logging gets init before managers we need to reapply the color scheme.
	LoggingapplyTheme();
	loadFrame(wxDEFAULT_FRAME_STYLE);

	if (!m_bQuiteMode)
		m_wxTBIcon = new TaskBarIcon(this);

	std::string szAppid = UTIL::OS::getConfigValue(APPID);

	uint32 appid = -1;

	if (szAppid.size() > 0)
		appid = atoi(szAppid.c_str());

	if (appid == BUILDID_BETA || appid == BUILDID_INTERNAL)
		gc_uploaddumps.setValue(true);
}

void MainApp::run()
{
	showLogin();
}

void MainApp::disableQuietMode()
{
	if (!m_wxTBIcon)
	{
		m_wxTBIcon = new TaskBarIcon(this);
		m_wxTBIcon->regEvents();
	}

	m_bQuiteMode = false;
}

bool MainApp::isQuietMode()
{
	return m_bQuiteMode;
}

void MainApp::logIn(const char* user, const char* pass)
{
	Thread::AutoLock a(m_UserLock);

	safe_delete(g_pUserHandle);

	gcString path = UTIL::OS::getAppDataPath();

	g_pUserHandle = (UserCore::UserI*)UserCore::FactoryBuilderUC(USERCORE);
	g_pUserHandle->init(path.c_str());


	try
	{
		//need to do this here as news items will be passed onlogin
		*g_pUserHandle->getNewsUpdateEvent() += delegate(this, &MainApp::onNewsUpdate);
		*g_pUserHandle->getGiftUpdateEvent() += delegate(this, &MainApp::onGiftUpdate);
		*g_pUserHandle->getNeedCvarEvent() += delegate(this, &MainApp::onNeedCvar);

		g_pUserHandle->lockDelete();
		g_pUserHandle->logIn(user, pass);
		g_pUserHandle->unlockDelete();

#ifndef DEBUG
		if (gc_enable_api_debugging.getBool())
			g_pUserHandle->getWebCore()->enableDebugging();
#endif
	}
	catch (gcException)
	{
		g_pUserHandle->logOut();

		*g_pUserHandle->getNewsUpdateEvent() -= delegate(this, &MainApp::onNewsUpdate);
		*g_pUserHandle->getGiftUpdateEvent() -= delegate(this, &MainApp::onGiftUpdate);
		*g_pUserHandle->getNeedCvarEvent() -= delegate(this, &MainApp::onNeedCvar);

		g_pUserHandle->unlockDelete();
		safe_delete(g_pUserHandle);
		throw;
	}
}

void MainApp::logOut(bool bShowLogin, bool autoLogin)
{
	m_pInternalLink->closeAll();
	safe_delete(m_pInternalLink);

	{
		Thread::AutoLock a(m_UserLock);
		if (g_pUserHandle)
		{
			UserCore::UserI* user = g_pUserHandle;
			g_pUserHandle = NULL;

			user->logOut(!autoLogin);

			*user->getAppUpdateProgEvent()				-= guiDelegate(this, &MainApp::onAppUpdateProg);
			*user->getAppUpdateCompleteEvent()			-= guiDelegate(this, &MainApp::onAppUpdate);
			*user->getWebCore()->getCookieUpdateEvent() -= guiDelegate(this, &MainApp::onCookieUpdate);
			*user->getPipeDisconnectEvent()				-= guiDelegate(this, &MainApp::onPipeDisconnect);

			safe_delete(user);
		}
	}

	GetCVarManager()->saveAll();
	GetCVarManager()->cleanUserCvars();

	closeMainForm();

	m_bLoggedIn = false;
	m_iMode = MODE_UNINT;

	if (bShowLogin)
		showLogin(!autoLogin);

	HideLogForm();

	DeleteCookies();
	SetCrashDumpSettings(NULL, true);
}

void MainApp::goOffline()
{
	if (!m_pOfflineDialog)
		m_pOfflineDialog = new gcMessageDialog(NULL, Managers::GetString(L"#MF_OFFLINE"), Managers::GetString(L"#MF_OFFLINE_TITLE"), wxYES_NO | wxICON_QUESTION);

	int ans = m_pOfflineDialog->ShowModal();

	if (m_pOfflineDialog)
	{
		m_pOfflineDialog->Destroy();
		m_pOfflineDialog = NULL;

		if (ans == wxID_YES)
			offlineMode();
	}
}

bool MainApp::isOffline()
{
	return (m_iMode == MODE_OFFLINE);
}

bool MainApp::isLoggedIn()
{
	return (m_iMode == MODE_ONLINE);
}

void MainApp::offlineMode()
{
	if (m_iMode == MODE_OFFLINE)
		return;

	if (m_bLoggedIn)
		logOut(false);

	closeMainForm();

	{
		Thread::AutoLock a(m_UserLock);

		gcString path = UTIL::OS::getAppDataPath();
		safe_delete(g_pUserHandle);
		g_pUserHandle = (UserCore::UserI*)UserCore::FactoryBuilderUC(USERCORE);
		g_pUserHandle->init(path.c_str());

		try
		{
			*g_pUserHandle->getNeedCvarEvent() += delegate(this, &MainApp::onNeedCvar);
			g_pUserHandle->getItemManager()->loadItems();
		}
		catch (gcException &)
		{
			*g_pUserHandle->getNeedCvarEvent() -= delegate(this, &MainApp::onNeedCvar);
			g_pUserHandle->logOut();
			safe_delete(g_pUserHandle);
			throw;
		}
	}

	GetCVarManager()->loadUser(GetUserCore()->getUserId());
	m_iMode = MODE_OFFLINE;

	showMainWindow();
	m_pInternalLink = new InternalLink(this);
}

void MainApp::showMainWindow(bool raise)
{
	if (!m_bLoggedIn && m_iMode != MODE_OFFLINE)
	{
		showLogin();
	}
	else
	{
		showMainForm(raise);
	}
}


void MainApp::onClose(wxCloseEvent& event)
{
	if (m_wxLoginForm)
	{
		m_wxLoginForm->Show(false);
		m_wxLoginForm->Close(true);
	}

	if (m_wxMainForm)
	{
		m_wxMainForm->Show(false);
		m_wxMainForm->Close(true);
	}

	HideLogForm();

	if (m_pInternalLink)
		m_pInternalLink->closeAll();

	Destroy();
}



void MainApp::closeMainForm()
{
	if (!m_wxMainForm)
		return;

	wxFrame* temp = m_wxMainForm;
	m_wxMainForm = NULL;

	temp->Show(false);
	temp->Close(true);
	temp->Destroy();
}

EventV* MainApp::getLoginEvent()
{
	return &onLoginEvent;
}

void MainApp::onLoginAccepted(bool saveLoginInfo, bool autologin)
{
	std::pair<bool,bool> res(saveLoginInfo, autologin);
	onLoginAcceptedEvent(res);
}

void MainApp::onLoginAcceptedCB(std::pair<bool,bool> &loginInfo)
{
	bool saveLoginInfo = loginInfo.first;
	bool autologin = loginInfo.second;

	if (m_wxLoginForm)
	{
		m_wxLoginForm->Show(false);
		m_wxLoginForm->Destroy();
		m_wxLoginForm = NULL;
	}

	if (saveLoginInfo)
		GetUserCore()->saveLoginInfo();

	*GetUserCore()->getAppUpdateProgEvent() += guiDelegate(this, &MainApp::onAppUpdateProg);
	*GetUserCore()->getAppUpdateCompleteEvent() += guiDelegate(this, &MainApp::onAppUpdate);
	*GetWebCore()->getCookieUpdateEvent() += guiDelegate(this, &MainApp::onCookieUpdate);
	*GetUserCore()->getPipeDisconnectEvent() += guiDelegate(this, &MainApp::onPipeDisconnect);
	
	//trigger this so it sets cookies first time around
	onCookieUpdate();


	admin_developer.setValue(GetUserCore()->isAdmin());
	GetCVarManager()->loadUser(GetUserCore()->getUserId());
	
	gcWString userName(GetUserCore()->getUserName());
	SetCrashDumpSettings(userName.c_str(), gc_uploaddumps.getBool());

	m_bLoggedIn = true;
	m_iMode = MODE_ONLINE;

	bool showMain = !(autologin && gc_silentlaunch.getBool());
	showMainForm(false, showMain);

	m_pInternalLink = new InternalLink(this);

	if (!m_bQuiteMode)
		GetUserCore()->getItemManager()->checkItems();

	if (m_wxTBIcon)
		m_wxTBIcon->regEvents();

	if (showMain && !m_bQuiteMode)
		showNews();

	if (m_szDesuraCache != "")
	{
		m_pInternalLink->handleInternalLink(m_szDesuraCache.c_str());
		m_szDesuraCache = "";
	}

	if (!gc_updateduninstall.getBool())
	{
		GetUserCore()->updateUninstallInfo();
		gc_updateduninstall.setValue(true);
	}

	onLoginEvent();
}

#ifdef NIX
void MainApp::toggleCurrentForm()
{
	if (!m_wxMainForm && !m_wxLoginForm)
		return;
		
	if (m_wxMainForm)
	{
		if (m_wxMainForm->IsShown())
			m_wxMainForm->Show(false);
		else
			m_wxMainForm->forceRaise();
	}
	else if (m_wxLoginForm)
	{
		if (m_wxLoginForm->IsShown())
		{
			m_wxLoginForm->Show(false);
		}
		else
		{
			m_wxLoginForm->Show(true);
			m_wxLoginForm->Raise();
		}
	}
}
#endif

void MainApp::showMainForm(bool raise, bool show)
{
	if (m_wxLoginForm)
	{
		m_wxLoginForm->Show(false);
		m_wxLoginForm->Destroy();
		m_wxLoginForm = NULL;
	}

	if (!m_wxMainForm)
	{
		if (m_iMode == MODE_OFFLINE)
			m_wxMainForm = new MainForm(this, true);
		else
			m_wxMainForm = new MainForm(this);
	}

	if (m_wxMainForm->IsIconized())
	{
		m_wxMainForm->Iconize(false);
	}


	m_wxMainForm->setMode(m_iMode);

	if (!m_bQuiteMode)
		m_wxMainForm->Show(show);

	if (!m_bQuiteMode && raise)
		m_wxMainForm->forceRaise();
}

void MainApp::showLogin(bool skipAutoLogin)
{
#ifdef DEBUG
	showUnitTest();
#endif

	if (!m_wxLoginForm)
		m_wxLoginForm = new LoginForm(this);

	if (!skipAutoLogin && gc_savelogin.getBool())
		m_wxLoginForm->autoLogin();

	m_wxLoginForm->Show(!m_bQuiteMode);

	if (m_wxLoginForm->IsShown())
		m_wxLoginForm->Raise();

}

void MainApp::onAppUpdateProg(uint32& prog)
{
	if (prog == 0 || prog == 100)
	{
		setProgressState(P_NONE);
	}
	else
	{
		setProgressState(P_NORMAL);
		setProgress((uint8)prog);
	}
}

void MainApp::onAppUpdate(UserCore::Misc::UpdateInfo& info)
{
	setProgressState(P_NONE);
	m_pInternalLink->showAppUpdate(info.branch);
}

void MainApp::onNewsUpdate(std::vector<UserCore::Misc::NewsItem*>& itemList)
{
	m_NewsLock.lock();

	for (size_t x=0; x<itemList.size(); x++)
	{
		if (!itemList[x])
			continue;

		UserCore::Misc::NewsItem* temp = new UserCore::Misc::NewsItem(itemList[x]);
		m_vNewsItems.push_back(temp);
	}

	m_NewsLock.unlock();
}

void MainApp::onNotifyGiftUpdate()
{
	m_wxTBIcon->showGiftPopup(m_vGiftItems);
}

void MainApp::onGiftUpdate(std::vector<UserCore::Misc::NewsItem*>& itemList)
{
	std::vector<UserCore::Misc::NewsItem*> oldList;

	m_NewsLock.lock();

	oldList = m_vGiftItems;
	m_vGiftItems.clear();

	for (size_t x=0; x<itemList.size(); x++)
	{
		if (!itemList[x])
			continue;

		m_vGiftItems.push_back(new UserCore::Misc::NewsItem(itemList[x]));
	}

	size_t count = 0;

	for (size_t x=0; x<itemList.size(); x++)
	{
		for (size_t y=0; y<oldList.size(); y++)
		{
			if (oldList[y]->id == itemList[x]->id)
			{
				count++;
				oldList[y]->hasBeenShown = true;
				break;
			}
		}
	}

	m_NewsLock.unlock();

	safe_delete(oldList);

	if (m_vGiftItems.size() != count)
		onNotifyGiftUpdateEvent();
}

void MainApp::onNeedCvar(UserCore::Misc::CVar_s& info)
{
	if (info.name)
	{
		CVar* c = GetCVarManager()->findCVar(info.name);

		if (c)
			info.value = c->getString();
	}
}

void MainApp::onCookieUpdate()
{
	SetCookies();
}

void MainApp::onPipeDisconnect()
{
	DesuraServiceError dse(getMainWindow());
	dse.ShowModal();
}

void MainApp::newAccountLogin(const char* username, const char* cookie)
{
	if (!m_bLoggedIn && m_iMode != MODE_OFFLINE && m_wxLoginForm)
		m_wxLoginForm->newAccountLogin(username, cookie);
}

void MainApp::showUnitTest()
{
#ifdef WITH_GTEST
	if (!m_UnitTestForm)
		m_UnitTestForm = new gcUnitTestForm(this);

	m_UnitTestForm->postShowEvent();
#endif
}
