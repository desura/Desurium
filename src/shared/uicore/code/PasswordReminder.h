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

#ifndef DESURA_PASSREMINDER_H
#define DESURA_PASSREMINDER_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"


class PassReminderThread;

class PasswordReminder : public gcDialog
{
public:
	PasswordReminder(wxWindow* parent);
	~PasswordReminder();
	
#ifdef NIX
	bool Show(const bool show = true);
#endif
protected:
	void onTextBoxEnter( wxCommandEvent& event );
	void onButtonClicked( wxCommandEvent& event );
	bool isEmailValid();

	void onComplete();
	void onError(gcException& e);

private:
	wxStaticText* m_staticText1;
	wxStaticText* m_staticText2;

	gcTextCtrl* m_tbEmail;
	
	gcButton* m_butSend;
	gcButton* m_butCancel;

	PassReminderThread *m_pThread;
};

#endif //DESURA_PASSREMINDER_H
