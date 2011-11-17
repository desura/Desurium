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

#ifndef DESURA_GAMEDISCFORM_H
#define DESURA_GAMEDISCFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

namespace UserCore
{
	namespace Item
	{
		class ItemInfoI;
	}
}

class GameDiskForm : public gcFrame 
{
public:
	GameDiskForm(wxWindow* parent, const char* exe, bool cdkey);
	~GameDiskForm();

	void setInfo(DesuraId id);

	DesuraId getItemId();

protected:
	wxStaticText* m_labInfo;
	gcCheckBox* m_cbReminder;

	gcButton* m_butLaunch;
	gcButton* m_butCancel;
	

	void onFormClose(wxCloseEvent& event);
	void onButtonClick(wxCommandEvent& event);

private:
	gcString m_szExe;
	bool m_bHasSeenCDKey;

	UserCore::Item::ItemInfoI* m_pItem;

};

#endif //DESURA_GAMEDISCFORM_H
