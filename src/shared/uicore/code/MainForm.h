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
#ifndef DESURA_MAIN_FORM_H
#define DESURA_MAIN_FORM_H

#ifdef _WIN32
#pragma once
#endif

#include "MainAppI.h"
#include "wx_controls/gcControls.h"


class CustomInstallPath;
class MainFormLeftBorder;
class AboutForm;
class DesuraControl;
class baseTabPage;

///////////////////////////////////////////////////////////////////////////////
/// Class MainForm
///////////////////////////////////////////////////////////////////////////////
class MainForm : public gcFrame
{
public:
	MainForm(wxWindow* parent, bool offline, const char* szProvider);
	~MainForm();

	virtual void loadFrame(long style);

	void setMode(uint8 mode);

	void onSubFormClose(uint32 eventId);
	void showAbout();
	void logOut();
	void goOffline();
	void Exit();

	void showPlay();
	void showPage(PAGE page);

	void loadUrl(const char* url, PAGE page);
	void onMenuSelect(wxCommandEvent& event );

	virtual void Maximize(bool state=true);

	void forceRaise();

protected:
	void showChangeLog();
	void createMenu(uint8 mode);

	void onFormClose(wxCloseEvent& event);
	void onMaximize(wxMaximizeEvent &event);
	void onIconize(wxIconizeEvent& event);

	void initPages(bool offline);
	void setTitle(bool offline);

private:
	AboutForm* m_wxAboutForm;
	DesuraControl* m_pDesuraControl;

	uint8 m_iMode;
	std::vector<baseTabPage*> m_vPageList;
};

#endif //DESURA_MAIN_FORM_H
