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
#include "LoginForm.h"

#include "MainApp.h"
#include "PasswordReminder.h"
#include "StripMenuButton.h"
#include "NewAccountDialog.h"

#include "managers/CVar.h"
#include <wx/msgdlg.h>

#include <branding/branding.h>

bool validateUsernameChange(const CVar*, const char*);

CVar gc_savelogin("gc_savelogin", "0");
CVar gc_saveusername("gc_saveusername", "1");

CVar gc_lastusername("gc_lastusername", "", CFLAG_NOFLAGS, &validateUsernameChange);
CVar gc_lastavatar("gc_lastavatar", "");

CVar gc_login_x("gc_login_x","-1", CFLAG_WINUSER);
CVar gc_login_y("gc_login_y","-1", CFLAG_WINUSER);


bool validateUsernameChange(const CVar* cvar, const char* newValue)
{
	if (gcString(newValue) == gcString(cvar->getString()))
		return false;

	gc_lastavatar.setValue("");
	return true;
}


#define GetHrgnOf(rgn)          ((HRGN)(rgn).GetHRGN())

///////////////////////////////////////////////////////////////////////////////
/// Login Thread
///////////////////////////////////////////////////////////////////////////////

class LoginThread : public Thread::BaseThread
{
public:
	LoginThread(const char* username, const char* password, LoginForm* caller) : BaseThread( "Login Thread")
	{
		m_szUsername = username;
		m_szPassword = password;

		m_pCaller = caller;
	}

	~LoginThread()
	{
		stop();
	}

protected:
	void run()
	{
		try
		{
			g_pMainApp->logIn(m_szUsername.c_str(), m_szPassword.c_str());

			if (m_pCaller)
				m_pCaller->onLoginEvent();
		}
		catch (gcException &except)
		{
			if (m_pCaller)
				m_pCaller->onLoginErrorEvent(except);
		}
	}

	gcString m_szUsername;
	gcString m_szPassword;

private:
	LoginForm* m_pCaller;
};



class LoginLink : public StripMenuButton
{
public:
	LoginLink(wxWindow* parent, wxString text, uint32 style) : StripMenuButton(parent, text)
	{
		wxColor linkColor = (wxColour)GetThemeManager().getColor("formlogin", "link-fg");
		wxColor hoverColor = (wxColour)GetThemeManager().getColor("formlogin", "link-hov-fg");

		Managers::LoadTheme(this, "formlogin");

		SetWindowStyle(style);

		setColors(linkColor, hoverColor);
		showFocusBox(hoverColor);
		init(NULL);

		Bind(wxEVT_CHAR, &LoginLink::onChar, this);
	}

protected:
	void onChar(wxKeyEvent &e)
	{
		if (e.m_keyCode == WXK_RETURN)
			sendClickCommand();
		else
			e.Skip();
	}
};
#ifdef NIX
static CVar gc_allow_wm_positioning("gc_allow_wm_positioning", "true");
#endif


///////////////////////////////////////////////////////////////////////////////
/// Class LoginForm
///////////////////////////////////////////////////////////////////////////////


LoginForm::LoginForm(wxWindow* parent) : gcFrame(parent, wxID_ANY, Managers::GetString(L"#LF_TITLE"), wxDefaultPosition, wxSize(420,246), wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxWANTS_CHARS|wxMINIMIZE_BOX, true)
{
	m_bAutoLogin = false;
	m_pNewAccount = NULL;

	Bind(wxEVT_COMMAND_TEXT_ENTER, &LoginForm::onTextBoxEnter, this);
	Bind(wxEVT_CLOSE_WINDOW, &LoginForm::onClose, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoginForm::onButtonClick, this);
	Bind(wxEVT_MOVE, &LoginForm::onMove, this);


	m_imgLogo = new gcImageControl(this, wxID_ANY, wxDefaultPosition, wxSize( 253,81 ));
	m_imgAvatar = new gcImageControl(this, wxID_ANY, wxDefaultPosition, wxSize( 66,66 ));

	m_imgLogo->setImage("#login_logo");
	m_imgAvatar->setImage("#login_default_avatar");

	m_tbUsername = new gcTextCtrl(this, wxID_ANY, Managers::GetString(L"#LF_USER"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxWANTS_CHARS);
	m_tbPassword = new gcTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxWANTS_CHARS|wxTE_PASSWORD);
	m_tbPasswordDisp = new gcTextCtrl(this, wxID_ANY, Managers::GetString(L"#LF_PASS"));

	m_cbRemPass = new gcCheckBox(this, wxID_ANY, Managers::GetString(L"#LF_AUTO"));
	m_cbRemPass->SetToolTip(Managers::GetString(L"#LF_AUTO_TOOLTIP"));

	m_butSignin = new gcButton(this, wxID_ANY, Managers::GetString(L"#LOGIN"), wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	m_butCancel = new gcButton(this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	m_linkOffline = new LoginLink(this, Managers::GetString(L"#LF_OFFLINE"), wxALIGN_LEFT);
	m_linkNewAccount = new LoginLink(this, Managers::GetString(L"#LF_NEWACCOUNT"), wxALIGN_CENTER);
	m_linkLostPassword = new LoginLink( this, Managers::GetString(L"#LF_LOSTPASS"), wxALIGN_RIGHT);

	//focus gets set to the first child. Redirect input to username box
	m_imgLogo->Bind(wxEVT_CHAR, &LoginForm::onFormChar, this);
	m_imgAvatar->Bind(wxEVT_CHAR, &LoginForm::onFormChar, this);

#ifdef WIN32
	Bind(wxEVT_LEFT_UP, &LoginForm::onMouseUp, this);
	Bind(wxEVT_LEFT_DOWN, &LoginForm::onMouseDown, this);
	Bind(wxEVT_MOTION, &LoginForm::onMouseMove, this);

	m_imgLogo->Bind(wxEVT_LEFT_UP, &LoginForm::onMouseUp, this);
	m_imgLogo->Bind(wxEVT_LEFT_DOWN, &LoginForm::onMouseDown, this);
	m_imgLogo->Bind(wxEVT_MOTION, &LoginForm::onMouseMove, this);

	m_imgAvatar->Bind(wxEVT_LEFT_UP, &LoginForm::onMouseUp, this);
	m_imgAvatar->Bind(wxEVT_LEFT_DOWN, &LoginForm::onMouseDown, this);
	m_imgAvatar->Bind(wxEVT_MOTION, &LoginForm::onMouseMove, this);
#endif

	m_tbUsername->Bind(wxEVT_COMMAND_TEXT_UPDATED, &LoginForm::onTextChange, this);
	m_tbUsername->Bind(wxEVT_CHAR, &LoginForm::onChar, this);
	m_tbUsername->Bind(wxEVT_KILL_FOCUS, &LoginForm::onBlur, this);
	m_tbUsername->Bind(wxEVT_SET_FOCUS, &LoginForm::onFocus, this);

	m_tbPassword->Bind(wxEVT_CHAR, &LoginForm::onChar, this);
	m_tbPassword->Bind(wxEVT_KILL_FOCUS, &LoginForm::onBlur, this);

	m_tbPasswordDisp->Bind(wxEVT_COMMAND_TEXT_UPDATED, &LoginForm::onTextChange, this);
	m_tbPasswordDisp->Bind(wxEVT_SET_FOCUS, &LoginForm::onFocus, this);

	m_cbRemPass->Bind(wxEVT_CHAR, &LoginForm::onChar, this);

	m_linkOffline->Bind(wxEVT_CHAR, &LoginForm::onChar, this);
	m_linkNewAccount->Bind(wxEVT_CHAR, &LoginForm::onChar, this);
	m_linkLostPassword->Bind(wxEVT_CHAR, &LoginForm::onChar, this);

	m_linkOffline->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoginForm::onLinkClick, this);
	m_linkNewAccount->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoginForm::onLinkClick, this);
	m_linkLostPassword->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoginForm::onLinkClick, this);


	m_butCancel->Bind(wxEVT_CHAR, &LoginForm::onChar, this);
	m_butSignin->Bind(wxEVT_CHAR, &LoginForm::onChar, this);

	m_tbUsername->SetValue(Managers::GetString(L"#LF_USER"));
	m_tbPasswordDisp->SetValue(Managers::GetString(L"#LF_PASS"));

	m_tbPassword->Show(false);
	m_tbPasswordDisp->Show(true);


	wxFlexGridSizer* fgSizer4 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer4->AddGrowableCol( 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer4->Add( m_cbRemPass, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	fgSizer4->Add( m_butSignin, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	fgSizer4->Add( m_butCancel, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );


	wxFlexGridSizer* fgSizer5 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer5->AddGrowableCol( 0 );
	fgSizer5->AddGrowableCol( 1 );
	fgSizer5->AddGrowableCol( 2 );
	fgSizer5->AddGrowableRow( 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer5->Add( m_linkOffline, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
	fgSizer5->Add( m_linkNewAccount, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	fgSizer5->Add( m_linkLostPassword, 0, wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );


	wxFlexGridSizer* fgSizer6 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer6->Add( m_tbUsername, 0, wxEXPAND|wxLEFT, 5 );
	fgSizer6->Add( m_tbPassword, 0, wxEXPAND|wxTOP|wxLEFT, 5 );
	fgSizer6->Add( m_tbPasswordDisp, 0, wxEXPAND|wxTOP|wxLEFT, 5 );
	fgSizer6->Add( fgSizer4, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer3 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer3->Add( m_imgAvatar, 0, 0, 5 );
	fgSizer3->Add( fgSizer6, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );


	wxFlexGridSizer* fgSizer2 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer2->Add( m_imgLogo, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );
	fgSizer2->Add( fgSizer3, 1, wxEXPAND, 5 );
	fgSizer2->Add( 0, 9, 1, wxEXPAND, 5 );
	fgSizer2->Add( fgSizer5, 1, wxEXPAND, 5 );


	wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableCol( 2 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( fgSizer2, 1, wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();


	m_bSavePos = false;

	m_vTabOrder.push_back(m_tbUsername);
	m_vTabOrder.push_back(m_tbPasswordDisp);
	m_vTabOrder.push_back(m_tbPassword);
	m_vTabOrder.push_back(m_cbRemPass);
	m_vTabOrder.push_back(m_butSignin);
	m_vTabOrder.push_back(m_butCancel);
	m_vTabOrder.push_back(m_linkOffline);
	m_vTabOrder.push_back(m_linkNewAccount);
	m_vTabOrder.push_back(m_linkLostPassword);

	Centre(wxCENTRE_ON_SCREEN);
	SetFocus();

	if (gc_login_x.getInt() == -1 || gc_login_y.getInt() == -1)
	{
		int x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		int y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

		int w=0;
		int h=0;
		GetSize(&w, &h);

		int newX = (x-w)/2;
		int newY = (y-h)/2;

		if (newX>0 && newY>0)
			SetSize(newX, newY, -1, -1);
	}
	else
	{
#ifdef NIX
		if (! gc_allow_wm_positioning.getBool())
#endif
			setSavedWindowPos(gc_login_x.getInt(), gc_login_y.getInt(), UINT_MAX, UINT_MAX);
	}

	if (gc_savelogin.getBool())
		m_cbRemPass->SetValue(true);

	if (gc_saveusername.getBool())
	{
		const char* str = gc_lastusername.getString();
		const char* avatar = gc_lastavatar.getString();

		if (gcString(str) != "")
		{
			m_tbUsername->SetValue(str);

			m_tbPasswordDisp->Show(false);
			m_tbPassword->Show();
			m_tbPassword->SetFocus();
			Layout();
		}

		if (avatar && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(avatar)))
			m_imgAvatar->setImage(avatar);
	}

	m_pLogThread = NULL;
	m_bSavePos = true;
	m_bMouseDrag = false;

	onLoginEvent += guiDelegate(this, &LoginForm::onLogin);
	onLoginErrorEvent += guiDelegate(this, &LoginForm::onLoginError);
	onStartLoginEvent += guiDelegate(this, &LoginForm::onStartLogin);

	Managers::LoadTheme(this, "formlogin");

	Managers::LoadTheme(m_cbRemPass, "formlogin");

#ifdef WIN32
	SetSize(wxSize(420,246));
#endif

	setFrameRegion();
}

LoginForm::~LoginForm()
{
	onLoginEvent -= guiDelegate(this, &LoginForm::onLogin);
	onLoginErrorEvent -= guiDelegate(this, &LoginForm::onLoginError);
	onStartLoginEvent -= guiDelegate(this, &LoginForm::onStartLogin);

	safe_delete(m_pLogThread);
}

wxRect LoginForm::getWindowsBorderRect() const
{
	// SetWindowRgn expects the region to be in coordinants
	// relative to the window, not the client area.  Figure
	// out the offset, if any.
#ifdef WIN32
	RECT rect;
	DWORD dwStyle =   ::GetWindowLong((HWND)GetHWND(), GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong((HWND)GetHWND(), GWL_EXSTYLE);
	::GetClientRect((HWND)GetHWND(), &rect);
	::AdjustWindowRectEx(&rect, dwStyle, ::GetMenu((HWND)GetHWND()) != NULL, dwExStyle);

	int x=rect.left*-1;
	int y=rect.top*-1;

	int w = rect.right+rect.left;
	int h = rect.bottom+rect.left;//+ (y-x)-1;

	wxPoint point = GetPosition();

	return wxRect(wxPoint(x, y), wxSize(w, h));
#else
	return wxRect(wxPoint(0,0), wxSize(800, 600));
#endif // LINUX TODO

}

void LoginForm::setFrameRegion()
{
#ifdef WIN32
	wxRect wbr = getWindowsBorderRect();
	wxPoint br = wbr.GetBottomRight();

	br.x += 2;
	br.y += 2;

	wxRegion region = CreateRoundRectRgn(wbr.x, wbr.y, br.x, br.y, 5, 5);

	// Windows takes ownership of the region, so
	// we'll have to make a copy of the region to give to it.
	DWORD noBytes = ::GetRegionData(GetHrgnOf(region), 0, NULL);
	RGNDATA *rgnData = (RGNDATA*) new char[noBytes];
	::GetRegionData(GetHrgnOf(region), noBytes, rgnData);
	HRGN hrgn = ::ExtCreateRegion(NULL, noBytes, rgnData);
	delete[] (char*) rgnData;

	// Now call the shape API with the new region.
	::SetWindowRgn((HWND)GetHWND(), hrgn, FALSE);
#endif // LINUX TODO
}



void LoginForm::onMouseUp(wxMouseEvent& event)
{
#ifdef WIN32
	if (!m_bMouseDrag)
		return;

	m_bMouseDrag = false;

	if (HasCapture())
		ReleaseMouse();
#else
	event.Skip();
#endif
}

void LoginForm::onMouseDown(wxMouseEvent& event)
{
#ifdef WIN32
	wxPoint pos = wxGetMousePosition()-GetScreenPosition();

	m_bMouseDrag = true;
	m_StartDrag = pos;
	m_StartPos = wxGetMousePosition();

	if (!HasCapture())
		CaptureMouse();
#else
	event.Skip();
#endif
}

void LoginForm::onMouseMove(wxMouseEvent& event)
{
#ifdef WIN32
	if (!m_bMouseDrag)
		return;

	wxPoint pos = wxGetMousePosition();
	SetPosition(pos-m_StartDrag);
#else
	event.Skip();
#endif
}

void LoginForm::onBlur(wxFocusEvent& event)
{
	if (event.GetId() == m_tbUsername->GetId())
	{
		if (m_tbUsername->GetValue() == "")
			m_tbUsername->SetValue(Managers::GetString(L"#LF_USER"));
	}
	else if (event.GetId() == m_tbPassword->GetId())
	{
		if (m_tbPassword->GetValue() == "" && m_tbPassword->IsShown())
		{
			m_tbPassword->Show(false);
			m_tbPasswordDisp->Show(true);
			Layout();
			return;
		}
	}

	event.Skip();
}

void LoginForm::onFocus(wxFocusEvent& event)
{
	if (event.GetId() == m_tbUsername->GetId())
	{
		gcString defaultText = Managers::GetString(L"#LF_USER");

		if (m_tbUsername->GetValue() == defaultText)
			m_tbUsername->SetValue("");
	}
	else if (event.GetId() == m_tbPasswordDisp->GetId())
	{
		if (m_tbPasswordDisp->IsShown())
		{
			m_tbPasswordDisp->Show(false);
			m_tbPassword->Show();
			Layout();
			m_tbPassword->SetFocus();
			return;
		}
	}

	event.Skip();
}

void LoginForm::onTextChange(wxCommandEvent& event)
{
	gcWString defaultText;
	gcTextCtrl* textCtrl;

	if (event.GetId() == m_tbUsername->GetId())
	{
		textCtrl = m_tbUsername;
		defaultText = Managers::GetString(L"#LF_USER");

		if (gc_saveusername.getBool())
		{
			const char* str = gc_lastusername.getString();
			const char* avatar = gc_lastavatar.getString();

			if (gcString(str) != gcString(m_tbUsername->GetValue().c_str().AsWChar()))
			{
				m_imgAvatar->setImage("#login_default_avatar");
			}
			else
			{
				if (avatar && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(avatar)))
					m_imgAvatar->setImage(avatar);
			}
		}
	}
	else if (event.GetId() == m_tbPasswordDisp->GetId())
	{
		textCtrl = m_tbPasswordDisp;
		defaultText = Managers::GetString(L"#LF_PASS");
	}
	else
	{
		return;
	}

	const char* colorName = "tb-fg";

	if (textCtrl->GetValue() == defaultText)
		colorName = "tb-default-fg";

	wxColor color = wxColor(GetThemeManager().getColor("formlogin", colorName));

	textCtrl->SetForegroundColour(color);
	textCtrl->Refresh();
}



void LoginForm::autoLogin()
{
	char* user = NULL;
	char* pass = NULL;

	try
	{
		UserCoreGetLoginFN* getLogin = (UserCoreGetLoginFN*)UserCore::FactoryBuilderUC(USERCORE_GETLOGIN);

		if (!getLogin)
			throw gcException();

		getLogin(&user, &pass);

		if (user && pass)
		{
			m_tbUsername->SetValue(user);
			m_tbPassword->SetValue(pass);

			m_tbPasswordDisp->Show(false);
			m_tbPassword->Show();
			Layout();

			m_bAutoLogin = true;
			doLogin();
		}
	}
	catch (gcException &except)
	{
		Msg(gcString("Auto Login Failed: {0}\n", except));
	}

	safe_delete(user);
	safe_delete(pass);
}

void LoginForm::onMove(wxMoveEvent  &event)
{
	if (m_bSavePos)
	{
		gc_login_x.setValue(GetPosition().x);
		gc_login_y.setValue(GetPosition().y);
	}

	event.Skip();
}

void LoginForm::onTextBoxEnter( wxCommandEvent& event )
{
	doLogin();
}

void LoginForm::onClose( wxCloseEvent& event )
{
	Show(false);

	// if we can veto means it wasnt forced by the parent
	if (event.CanVeto() && GetParent())
		GetParent()->Close(true);
}

void LoginForm::onLinkClick(wxCommandEvent& event)
{
	if (event.GetId() == m_linkNewAccount->GetId())
	{
		onNewAccount();
	}
	else if (event.GetId() == m_linkLostPassword->GetId())
	{
		PasswordReminder wxPassReminderForm(this);
		wxPassReminderForm.ShowModal();
	}
	else if (event.GetId() == m_linkOffline->GetId())
	{
		if (GetParent())
		{
			Show(false);

			MainApp* temp = dynamic_cast<MainApp*>(GetParent());
			if (temp)
				temp->offlineMode();
		}
	}
}

void LoginForm::onButtonClick(wxCommandEvent& event)
{
	if (event.GetId() == m_butCancel->GetId())
	{
		Close();
	}
	else if (event.GetId() == m_butSignin->GetId())
	{
		doLogin();
	}
}

void LoginForm::doLogin()
{
	gcString user((const wchar_t*)m_tbUsername->GetValue().c_str());
	gcString pass((const wchar_t*)m_tbPassword->GetValue().c_str());

	if (user == "" || pass == "")
	{
		gcWString errMsg(L"{0}\n", Managers::GetString(L"#LF_VALDERROR"));

		if (user == "")
			errMsg += gcWString(L"\t{0}\n", Managers::GetString(L"#LF_ERRBLANKNAME"));

		if (pass == "")
			errMsg += gcWString(L"\t{0}\n", Managers::GetString(L"#LF_ERRPASSWORD"));

		errMsg += gcWString(L"\n{0}", Managers::GetString(L"#LF_ERRPLEASE"));
		gcMessageBox(this, errMsg, Managers::GetString(L"#LF_ERRTITLE"));

		return;
	}

	m_tbUsername->Disable();
	m_tbPassword->Disable();
	m_butSignin->Disable();

	m_cbRemPass->Disable();

	m_butCancel->Disable();

	m_linkOffline->Disable();
	m_linkNewAccount->Disable();
	m_linkLostPassword->Disable();

	Refresh(false);
	onStartLoginEvent();
}

void LoginForm::onStartLogin()
{
	safe_delete(m_pLogThread);

	wxString user = m_tbUsername->GetValue();
	wxString pass = m_tbPassword->GetValue();

	if (m_bAutoLogin) //password will be user id cookie for autologin
	{
		user = m_tbPassword->GetValue();
		pass = "";
	}

#ifdef WIN32
	m_pLogThread = new LoginThread(gcString(user.wc_str()).c_str(), gcString(pass.wc_str()).c_str(), this);
#else
	m_pLogThread = new LoginThread(user.c_str(), pass.c_str(), this);
#endif

	m_tbPassword->SetValue("****************");
	m_pLogThread->start();
}

void LoginForm::onLogin()
{
	safe_delete(m_pLogThread);

	bool remPass = m_cbRemPass->GetValue();
	gc_savelogin.setValue(remPass);

	if (gc_saveusername.getBool())
	{
		wxString dastr = m_tbUsername->GetValue();
		gc_lastusername.setValue((const char*)dastr.c_str());
	}

	Show(false);

	MainApp* temp = dynamic_cast<MainApp*>(GetParent());

	if (temp)
		temp->onLoginAccepted(remPass, m_bAutoLogin);
}

void LoginForm::onLoginError(gcException &e)
{
	safe_delete(m_pLogThread);

	if (!m_bAutoLogin)
		gcErrorBox(this, "#LF_ERRTITLE", "#LF_ERROR", e);
	else
		Msg(gcString("Auto login failed: {0}\n", e));

	m_tbPassword->Clear();

	m_tbUsername->Enable();
	m_tbPassword->Enable();
	m_butSignin->Enable();
	m_butCancel->Enable();

	m_cbRemPass->Enable();

	m_linkOffline->Enable();
	m_linkNewAccount->Enable();
	m_linkLostPassword->Enable();

	m_bAutoLogin = false;
}

void LoginForm::onChar(wxKeyEvent& event)
{
	if (event.m_keyCode == WXK_TAB)
	{
		processTab(!event.ShiftDown(), event.GetId());
	}
	else
	{
		if (event.GetId() == m_cbRemPass->GetId())
		{
			if (event.m_keyCode == WXK_RETURN)
				doLogin();
			else if (event.m_keyCode == ' ')
				m_cbRemPass->SetValue(!m_cbRemPass->GetValue());
		}

		event.Skip();
	}
}

void LoginForm::onFormChar(wxKeyEvent& event)
{
	if (event.m_keyCode == WXK_TAB)
	{
		processTab(!event.ShiftDown(), event.GetId());
	}
	else
	{
		int32 keyCode = event.GetKeyCode();
		if (!event.HasModifiers() && ((keyCode >= '0' && keyCode <= '9') || (keyCode >= 'a' && keyCode <= 'z') || (keyCode >= 'A' && keyCode <= 'Z' )))
		{
			m_tbUsername->SetFocus();
			m_tbUsername->SetValue((char)keyCode);
			m_tbUsername->SetInsertionPointEnd();
		}
	}
}

void LoginForm::processTab(bool forward, int32 id)
{
	uint32 index = UINT_MAX;

	for (size_t x=0; x<m_vTabOrder.size(); x++)
	{
		if (m_vTabOrder[x]->GetId() == id)
		{
			index = x;
			break;
		}
	};

	if (index != UINT_MAX)
	{
		uint32 start = index;

		do
		{
			if (forward)
			{
				index++;

				if (index == m_vTabOrder.size())
					index=0;
			}
			else
			{
				if (index == 0)
					index=m_vTabOrder.size();

				index--;
			}

			if (index == start)
				return;
		}
		while (m_vTabOrder[index]->IsShown() == false);

		m_vTabOrder[index]->SetFocus();

		wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(m_vTabOrder[index]);

		if (text)
			text->SetInsertionPointEnd();
	}
	else
	{
		m_tbUsername->SetFocus();
	}
}

void LoginForm::onNewAccount()
{
	NewAccountDialog naf(this);

	m_pNewAccount = &naf;
	naf.ShowModal();
	m_pNewAccount = NULL;
}

void LoginForm::newAccountLogin(const char* username, const char* cookie)
{
	if (!cookie)
		return;

	if (m_pNewAccount)
		m_pNewAccount->EndModal(0);

	m_tbUsername->SetValue(username);
	m_tbPassword->SetValue(cookie);

	m_tbPasswordDisp->Show(false);
	m_tbPassword->Show();
	Layout();

	doLogin();
}

