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

#ifndef DESURA_NEWSFORM_H
#define DESURA_NEWSFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "gcMiscWebControl.h"
#include "usercore/NewsItem.h"
#include "wx_controls/gcControls.h"
#include "wx/wx.h"


#ifdef ENABLE_OUTOFPROCESS_BROWSER
	typedef gcMiscWebHost NewsBrowser;
#else
	typedef gcMiscWebControl NewsBrowser;
#endif

class NewsForm : public gcFrame 
{
public:
	NewsForm(wxWindow* parent);
	~NewsForm();

	void loadNewsItems(std::vector<UserCore::Misc::NewsItem*> &itemList);
	void setAsGift();

protected:
	NewsBrowser* m_ieBrowser;
	
	gcButton* m_butPrev;
	gcButton* m_butNext;
	gcButton* m_butClose;

	void onFormClose( wxCloseEvent& event );
	void onButClick( wxCommandEvent& event );
	void loadSelection();

private:
	uint32 m_uiSelected;
	
	gcWString m_szTitle;
	gcWString m_szLoadingUrl;

	std::vector<UserCore::Misc::NewsItem*> m_vItemList;
};

#endif //DESURA_NEWSFORM_H
