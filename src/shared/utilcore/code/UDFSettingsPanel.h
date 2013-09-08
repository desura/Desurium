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

#ifndef DESURA_UDFSETTINGSPANEL_H
#define DESURA_UDFSETTINGSPANEL_H
#ifdef _WIN32
#pragma once
#endif


#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/panel.h>


enum UDF_FLAGS
{
	REMOVE_SIMPLE		= 1<<0,
	REMOVE_CACHE		= 1<<1,
	REMOVE_SETTINGS		= 1<<2,
};

class UDFSettingsPanel : public wxPanel 
{
public:
	UDFSettingsPanel(wxWindow* parent);
	~UDFSettingsPanel();

	uint32 getFlags();

protected:
	wxStaticText* m_labUninstallInfo;
	wxStaticText* m_labUninstallTitle;
	wxStaticText* m_labULocTitle;
	wxTextCtrl* m_tbUninstallLoc;

	wxCheckBox* m_cbRemoveDesura;
	wxCheckBox* m_cbRemoveSettings;
	wxCheckBox* m_cbRemoveCache;
	wxCheckBox* m_cbRemoveSimple;
	wxCheckBox* m_cbRemoveComplex;
		
	
private:
	
};

#endif //DESURA_UDFSETTINGSPANEL_H
