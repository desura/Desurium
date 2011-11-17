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

#ifndef DESURA_TASKBAR_ICON_H
#define DESURA_TASKBAR_ICON_H
#ifdef _WIN32
#pragma once
#endif


#include "wx_controls/gcTaskBar.h"
#include "wx_controls/gcCustomMenu.h"
#include "wx/menu.h"
#include "Managers.h"

#define MODCORE_ICON "appicon.ico"


class TBIBaseMenu;

class TaskBarIcon : public gcTaskBarIcon //, gcCustomMenu
{
friend class windowReg;

public:
	TaskBarIcon(wxWindow *parent);
	~TaskBarIcon();

	void regEvents();
	void deregEvents();

	void showGiftPopup(std::vector<UserCore::Misc::NewsItem*>& itemList);

protected:
	wxMenu* CreatePopupMenu();

	void onBallonClick(wxTaskBarIconEvent &event);
	void onMenuSelect( wxCommandEvent& event );
    void onLeftButtonDClick(wxTaskBarIconEvent& event);
#ifdef NIX
    void onLeftButtonDown(wxTaskBarIconEvent& event);
#endif
	void onUpdate();

	void onItemChanged(UserCore::Item::ItemInfoI::ItemInfo_s& info);

	void tagItems();
	void onItemsAdded(uint32&);
	void onUserUpdate();

	bool findUpdateItem(UserCore::Item::ItemInfoI* item);
	void removeUpdateItem(UserCore::Item::ItemInfoI* item);
	void swapUpdateList();

	void doBallonMsg();
	void updateIcon();

	uint32 getUpdateCount();

	void onAppUpdateComplete(UserCore::Misc::UpdateInfo &info);
	void onAppUpdate(UserCore::Misc::UpdateInfo &info);
	void onAppUpdateProg(uint32& prog);


private:
	void showMainWindow();

	uint32 m_iLastBallon;
	uint32 m_uiLastCount;
	uint32 m_uiLastProg;

	wxIcon m_wxIcon;
	wxWindow* m_wxParent;

	std::vector<UserCore::Item::ItemInfoI*> m_vNextUpdateList;
	std::vector<UserCore::Item::ItemInfoI*> m_vUpdatedList;

	UserCore::Misc::UpdateInfo m_AppUpdateVer;

	gcImageHandle m_imgCount;

#ifdef WIN32
	TBIBaseMenu* m_pModMenu;
#endif
	TBIBaseMenu* m_pGameMenu;
	TBIBaseMenu* m_pWindowMenu;
	TBIBaseMenu* m_pUpdateMenu;

	gcString m_szTooltip;

	friend void testapppopup_cc_func(std::vector<gcString> &vArgList);
};



#endif //DESURA_TASKBAR_ICON_H
