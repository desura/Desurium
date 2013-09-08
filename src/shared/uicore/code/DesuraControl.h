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

#ifndef DESURA_DESURACONTROL_H
#define DESURA_DESURACONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "MainAppI.h"
#include "wx_controls/wxEventDelegate.h"

class baseTabPage;
class MainMenuButton;
class MenuStrip;
class ButtonStrip;
class UsernameBox;
class gcStatusBar;

class BaseToolBarControl;

typedef struct
{
	int32 id;
	baseTabPage* page;
	BaseToolBarControl* header;
} tabInfo_s;

class baseTabPage;
class gcMainCustomFrameImpl;
class MainFormLeftBorder;
class DesuraMenuFiller;
class FrameButtons;

///////////////////////////////////////////////////////////////////////////////
/// Class tabPage
///////////////////////////////////////////////////////////////////////////////
class DesuraControl : public gcPanel 
{
public:
	DesuraControl(gcFrame* parent, bool offline);
	~DesuraControl();

	void addPage(baseTabPage *page, const char* tabName);

	void setActivePage(PAGE index, bool reset = false);
	void setActivePage_ID(int32 id);

	

	void regCustomFrame(gcMainCustomFrameImpl* mcf);

	void showLeftBorder(bool state);
	void refreshSearch();

	void setBaseTabPage(PAGE pageId, baseTabPage *page);
	PAGE getActivePage();

protected:
	wxBoxSizer* m_sizerHeader;
	wxFlexGridSizer* m_sizerContent;

	MainMenuButton *m_pMainMenuButton;
	MenuStrip *m_pMenuStrip;
	ButtonStrip *m_bButtonStrip;
	UsernameBox *m_pUsernameBox;
	DesuraMenuFiller *m_pFiller;
	FrameButtons* m_pFrameButtons;

	wxEventDelegateWrapper<gcImageButton> *m_pAvatar;

	void unloadSearch(baseTabPage* page);
	void loadSearch(baseTabPage* page);
	void updateStatusBar(uint32 index);

	void onButtonClicked( wxCommandEvent& event );
	void onMenuSelect( wxCommandEvent& event );
	void onExitPressed( wxMouseEvent& event );
	void onResize( wxSizeEvent& event );

	void onProgressUpdate(uint32& idInt);
	void onNewAvatar(gcString& image);
	void onDesuraUpdate(uint32& prog);

	void onActiveToggle(bool &state);

private:
	std::vector<tabInfo_s*> m_vTabInfo;
	uint32 m_iIndex;

	bool m_bDownloadingUpdate;
	bool m_bOffline;

	uint32 m_uiUpdateProgress;

	MainFormLeftBorder* m_pLeftBorder;
	gcMainCustomFrameImpl* m_pMainCustomFrame;

	DECLARE_EVENT_TABLE();
};



#endif //DESURA_DESURACONTROL_H
