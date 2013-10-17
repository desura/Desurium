/*
Desura is the leading indie game distribution platform
Copyright (C) Mark Chandler <mark@moddb.com>

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


#ifndef DESURA_NEWACCOUNT_H
#define DESURA_NEWACCOUNT_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcDialog.h"
#include "gcMiscWebControl.h"

class gcButton;

class NewAccountDialog : public gcDialog 
{
public:
	NewAccountDialog(wxWindow* parent);
	
#ifdef NIX
	bool Show(const bool show = true);
#endif

protected:
	void onButtonClicked(wxCommandEvent& event);
	void onPageLoad();

private:
	gcButton *m_butBack;
	gcButton *m_butAgree;
	gcButton *m_butCancel;

	gcMiscWebControl *m_pBrowser;

	bool m_bTermsOfService;
};



#endif //DESURA_NEWACCOUNT_H
