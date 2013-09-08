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

#ifndef DESURA_UNINSTALLDESURAFORM_H
#define DESURA_UNINSTALLDESURAFORM_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/frame.h>

namespace UserCore
{
	class UserI;
}


class UninstallForm : public wxFrame 
{
public:
	UninstallForm(wxWindow* parent, UserCore::UserI* user);
	~UninstallForm();

	void completedUninstall();

protected:
	wxPanel* m_pContent;

	wxStaticText* m_labTitle;
	wxStaticText* m_labTitleInfo;
	wxStaticBitmap* m_imgLogo;
	wxStaticLine* m_staticline1;
		
	wxButton* m_butRepair;
	wxButton* m_butUninstall;
	wxButton* m_butCancel;	

	wxBoxSizer* m_bContentSizer;

	void onButtonClicked(wxCommandEvent &event);

private:
	UserCore::UserI* m_pUser;
	bool m_bComplete;
};

#endif //DESURA_UNINSTALLDESURAPANEL_H
