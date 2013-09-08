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

#ifndef DESURA_EULAFORM_H
#define DESURA_EULAFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx/wx.h"
#include "gcMiscWebControl.h"

class EULAForm : public gcFrame 
{
public:
	EULAForm(wxWindow* parent);
	~EULAForm();

	bool setInfo(DesuraId id);
	DesuraId getItemId(){return m_uiInternId;}

protected:
	wxStaticText* m_labInfo;
	gcMiscWebControl* m_ieBrowser;

	gcButton* m_butAgree;
	gcButton* nm_butCancel;

	wxBoxSizer *m_BrowserSizer;

	void onButtonPressed(wxCommandEvent& event);
	void onFormClose( wxCloseEvent& event );

	void onNewUrl(newURL_s& info);
	void onPageLoad();

private:
	DesuraId m_uiInternId;
	bool m_bLoadingProperPage;
};

#endif //DESURA_EULAFORM_H
