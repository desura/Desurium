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


#ifndef DESURA_LOGINFORM_H
#define DESURA_LOGINFORM_H


#include <wx/gbsizer.h>

#include "Event.h"
#include "Managers.h"
#include "util_thread/BaseThread.h"
#include "wx_controls/gcControls.h"


class LoginThread;
class StripMenuButton;

///////////////////////////////////////////////////////////////////////////////
/// Class LoginForm
///////////////////////////////////////////////////////////////////////////////
class LoginForm : public gcFrame
{
public:
	LoginForm(wxWindow* parent);
	~LoginForm();

	void autoLogin();

	EventV onLoginEvent;
	Event<gcException> onLoginErrorEvent;

	void newAccountLogin(const char* username, const char* cookie);

protected:
	gcTextCtrl* m_tbUsername;
	gcTextCtrl* m_tbPassword;
	gcTextCtrl* m_tbPasswordDisp;

	gcCheckBox* m_cbRemPass;

	gcStaticLine* m_staticline1;

	gcButton* m_butSignin;
	gcButton* m_butCancel;

	gcImageControl* m_imgLogo;
	gcImageControl* m_imgAvatar;


	StripMenuButton* m_linkOffline;
	StripMenuButton* m_linkNewAccount;
	StripMenuButton* m_linkLostPassword;

	void doLogin();

	void onLogin();
	void onLoginError(gcException &e);

	void onStartLogin();
	EventV onStartLoginEvent;

	void processTab(bool forward, int32 id);

	wxRect getWindowsBorderRect() const;
	void setFrameRegion();

	void onButtonClick(wxCommandEvent& event);
	void onTextBoxEnter(wxCommandEvent& event);
	void onMove(wxMoveEvent& event);
	void onChar(wxKeyEvent& event);
	void onClose(wxCloseEvent& event);

	void onFormChar(wxKeyEvent& event);

	void onMouseUp(wxMouseEvent& event);
	void onMouseDown(wxMouseEvent& event);
	void onMouseMove(wxMouseEvent& event);
	void onTextChange(wxCommandEvent& event);
	void onFocus(wxFocusEvent& event);
	void onBlur(wxFocusEvent& event);

	void onLinkClick(wxCommandEvent& event);

	void onNewAccount();

private:
	bool m_bSavePos;
	LoginThread* m_pLogThread;

	bool m_bAutoLogin;

	std::vector<wxWindow*> m_vTabOrder;

	bool m_bMouseDrag;
	wxPoint m_StartPos;
	wxPoint m_StartDrag;

	wxDialog* m_pNewAccount;
};

#endif
