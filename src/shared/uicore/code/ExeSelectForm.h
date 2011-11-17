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

#ifndef DESURA_EXESELECTFORM_H
#define DESURA_EXESELECTFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

class ExeSelectForm : public gcFrame 
{
public:
	ExeSelectForm(wxWindow* parent, bool hasSeenCDKey);
	~ExeSelectForm();

	DesuraId getItemId(){return m_Id;}
	void setInfo(DesuraId id);

protected:
	wxStaticText* m_labInfo;
	wxFlexGridSizer* m_pButtonSizer;

	void onButtonClick(wxCommandEvent& event);
	void onFormClose(wxCloseEvent& event);

private:
	bool m_bHasSeenCDKey;

	std::vector<gcButton*> m_vButtonList;
	DesuraId m_Id;
};


#endif
