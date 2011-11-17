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


#ifndef DESURA_ABOUT_FORM_H
#define DESURA_ABOUT_FORM_H
#ifdef _WIN32
#pragma once
#endif


#include "wx_controls/gcControls.h"
#include <wx/hyperlink.h>

#include "Managers.h"

///////////////////////////////////////////////////////////////////////////

#define af_wxCLOSE 1000

class AboutMainPage;
class AboutDevPage;

///////////////////////////////////////////////////////////////////////////////
/// Class AboutForm
///////////////////////////////////////////////////////////////////////////////
class AboutForm : public gcDialog 
{
public:
	AboutForm(wxWindow* parent);
	~AboutForm();

protected:
	void onFormClose( wxCloseEvent& event );
	void onButtonClicked( wxCommandEvent& event );

private:
	gcImageControl* m_imgLogo;

	gcButton* m_butCredits;
	gcButton* m_butClose;

	AboutMainPage* m_pMainPage;
	AboutDevPage* m_pDevPage;

	wxBoxSizer* m_SizePanel;

	bool m_bMain;
};

#endif //DESURA_ABOUT_FORM_H
