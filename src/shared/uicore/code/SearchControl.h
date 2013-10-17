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

#ifndef DESURA_SEARCHCONTROL_H
#define DESURA_SEARCHCONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

class SearchControl : public gcPanel
{
public:
	SearchControl(wxWindow* parent);
	~SearchControl();

	gcImageButton* getButton();

	Event<gcString> onFullSearchEvent;
	Event<gcString> onSearchEvent;

	void onActiveToggle(bool state);
	void focusSearch();

protected:
	void onButtonClick(wxCommandEvent &e);
	void onSearchChanged(wxCommandEvent& event);
	void onDoSearch(wxCommandEvent& event);

	void onKeyDown(wxKeyEvent& event);

	void onBlur(wxFocusEvent& event);
	void onFocus(wxFocusEvent& event);

private:
	gcImageButton *m_butFullSearch;
	wxTextCtrl* m_tbSearchBox;
	gcWString m_szDefaultText;

	wxColor m_DefaultCol;
	wxColor m_NormalCol;
};


#endif //DESURA_SEARCHCONTROL_H
