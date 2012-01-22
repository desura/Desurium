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

#ifndef DESURA_MAINMENUBUTTON_H
#define DESURA_MAINMENUBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/control.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>

#include <wx/menu.h>
#include "BaseMenuButton.h"

#include "wx_controls/wxEventDelegate.h"
#include "wx_controls/gcCustomMenu.h"

enum
{
	DESURA_wxOffline = 1000,
	DESURA_wxLogOut,
	DESURA_wxSettings,
	DESURA_wxBackUp,
	DESURA_wxNewMCF,
	DESURA_wxOpenMCF,
	DESURA_wxInstallMCF,
	DESURA_wxModWizard,
	DESURA_wxBandwidthMon,
	DESURA_wxSpeedSched,
	DESURA_wxAbout,
	DESURA_wxHelp,
	DESURA_wxLog,
	DESURA_wxExit,
	DESURA_wxChangeLog,
	DESURA_wxCustomInstallPath,
	DESURA_wxListKeys,
	DESURA_wxDownloadReport,
	DESURA_wxCart,
	DESURA_wxPurchase,
	DESURA_wxGifts,
	DESURA_wxActivateGame,
};

class gcMainCustomFrameImpl;
class gcMenu;

class MainMenuButton : public gcImageButton
{
public:
	MainMenuButton(wxWindow* parent, bool offline);
	~MainMenuButton();

#ifdef WIN32
	virtual WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
	void onActiveToggle(bool state);

protected:
	void createMenu(bool offline = false);
	void onMouseClick(wxCommandEvent& event);
	void onBlur(wxFocusEvent& event);

private:
	gcMenu* m_mainMenu;
	bool m_bIgnoreNextClick;
};


#endif //DESURA_MAINMENUBUTTON_H
