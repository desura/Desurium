/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Mark Chandler <mark@moddb.com>

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

#ifndef DESURA_MAINAPP_H
#define DESURA_MAINAPP_H

#include "MainAppI.h"


#include "wx/app.h"
#include "webcore/WebCoreI.h"
#include "usercore/UserCoreI.h"
#include "usercore/UploadInfoThreadI.h"

class gcMessageDialog;

extern "C"
CEXPORT void* CreateMainApp();

extern bool isSafeUrl(const char* url);
extern WebCore::WebCoreI* GetWebCore();
extern UserCore::UserI* GetUserCore();

const char* GetAppVersion();
const char* GetUserCoreVersion();
const char* GetWebCoreVersion();

typedef struct
{
	gcString path;
	DesuraId id;
	wxWindow *caller;
} ut;

enum
{
	MODE_UNINT = 0,		//not yet setup
	MODE_ONLINE,		//normal client
	MODE_ONLINE_MIN,	//small client
	MODE_OFFLINE,		//offline client
	MODE_LOGOUT,		//logged out (no client)
};

class LoginForm;
class wxOnAppUpdateEvent;
class wxTaskBarIcon;
class MainForm;

namespace UserCore
{
	namespace Thread
	{
		class UserThreadI;
	}

	namespace Misc
	{
		class NewsItem;
	}
}

class InternalLinkInfo
{
public:
	DesuraId id;
	uint8 action;
	std::vector<std::string> args;
};

class InternalLink;
class TaskBarIcon;

class MainApp :  public MainAppI
{
public:
	MainApp();
	virtual ~MainApp();

	void run();
	//wx Function overide
	void Init(int argc, wxCmdLineArgsArray &argv);
	void showMainWindow(bool raise = false);

	void onTBIMenuSelect(wxCommandEvent& event);

	void onLoginAccepted(bool saveLoginInfo = false, bool autologin = false);

	void logIn(const char* user, const char* pass);
	void logOut(bool bShowLogin = true, bool autoLogin = false);

	void offlineMode();
	void goOffline();

	bool isOffline();
	bool isLoggedIn();

	void handleInternalLink(const char* link);
	void handleInternalLink(DesuraId id, uint8 action, std::vector<std::string> args);

	void closeMainForm();
	void closeForm(int32 id);
#ifdef NIX
	void toggleCurrentForm();
#endif

	void processWildCards(WCSpecialInfo &info, wxWindow* parent);

	gcFrame* getMainWindow();
	wxWindow* getTopLevelWindow();

	virtual void disableQuietMode();
	virtual bool isQuietMode();

	virtual EventV* getLoginEvent();


	virtual void newAccountLogin(const char* username, const char* cookie);

protected:
	void showLogin(bool skipAutoLogin = false);
	void showOffline();
	void showMainForm(bool raise = false, bool show = true);

	void createTaskBarIcon();
	void onClose(wxCloseEvent& event);
	
	void changeAccountState(DesuraId id);

	void loadUrl(const char* url, PAGE page);
	void showProfile(DesuraId id, std::vector<std::string> args = std::vector<std::string>());
	void showDevProfile(DesuraId id);
	void showDevPage(DesuraId id);
	void showNews();
	void showPlay();
	void showPage(PAGE page);

	void getSteamUser(WCSpecialInfo *info, wxWindow *parent);

	void onNewsUpdate(std::vector<UserCore::Misc::NewsItem*>& itemList);
	void onGiftUpdate(std::vector<UserCore::Misc::NewsItem*>& itemList);
	void onNeedCvar(UserCore::Misc::CVar_s& info);

	void onAppUpdateProg(uint32& prog);
	void onAppUpdate(UserCore::Misc::UpdateInfo& info);

	void onCookieUpdate();

	void onLoginAcceptedCB(std::pair<bool,bool> &loginInfo);

	void onPipeDisconnect();

	void onInternalStrLink(gcString &link);
	void onInternalLink(InternalLinkInfo& info);

	void onNotifyGiftUpdate();

	void showConsole();

	Event<std::pair<bool,bool>> onLoginAcceptedEvent;
	EventV onLoginEvent;

	Event<InternalLinkInfo> onInternalLinkEvent;
	Event<gcString> onInternalLinkStrEvent;

	EventV onNotifyGiftUpdateEvent;

private:
	gcMessageDialog *m_pOfflineDialog;

	std::vector<UserCore::Misc::NewsItem*> m_vNewsItems;
	std::vector<UserCore::Misc::NewsItem*> m_vGiftItems;

	UserCore::Thread::UserThreadI* m_pDumpThread;

	bool m_bQuiteMode;
	bool m_bLoggedIn;
	uint8 m_iMode;

	gcString m_szDesuraCache;

	LoginForm* m_wxLoginForm;
	MainForm* m_wxMainForm;
	TaskBarIcon* m_wxTBIcon; 

	InternalLink *m_pInternalLink;
	::Thread::Mutex m_NewsLock;

	::Thread::Mutex m_UserLock;

	friend void cc_NewsTest_cc_func(std::vector<gcString> &argv);
	friend void cc_GiftTest_cc_func(std::vector<gcString> &argv);

#ifdef DEBUG
	friend class PopUpThread;
#endif

	friend void cc_test_news_cc_func(std::vector<gcString> &vArgList);
};


extern MainAppI* g_pMainApp;

#endif
