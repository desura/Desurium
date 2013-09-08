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

#include "Common.h"
#include "PasswordReminder.h"

#include "webcore/WebCoreI.h"

class PassReminderThread : public Thread::BaseThread
{
public:
	PassReminderThread(const char* email) : BaseThread("Password Reminder Thread")
	{
		m_szEmail = gcString(email);
	}

	Event<gcException> onErrorEvent;
	EventV onCompleteEvent;

protected:
	void run()
	{
		try
		{
			PassReminderFN passReminder = (PassReminderFN)WebCore::FactoryBuilder(WEBCORE_PASSREMINDER);
			passReminder(m_szEmail.c_str());
			onCompleteEvent();
		}
		catch (gcException &e)
		{
			onErrorEvent(e);
		}
	}

private:
	gcString m_szEmail;
};

PasswordReminder::PasswordReminder(wxWindow* parent) : gcDialog(parent, wxID_ANY, wxT("#PR_TITLE"), wxDefaultPosition, wxSize( 370,134 ), wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PasswordReminder::onButtonClicked, this);
	Bind(wxEVT_COMMAND_TEXT_ENTER, &PasswordReminder::onTextBoxEnter, this);

	SetTitle(Managers::GetString(L"#PR_TITLE"));

	m_staticText1 = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#PR_PROMPT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( 360 );

	m_staticText2 = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#PR_EMAIL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );

	m_tbEmail = new gcTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );

	m_butSend = new gcButton( this, wxID_ANY, Managers::GetString(L"#SEND"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );


	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );
	bSizer2->Add( m_tbEmail, 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer3->Add( m_butSend, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer3->Add( m_butCancel, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 5, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 1 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer2->Add( m_staticText1, 0, wxALL, 5 );
	fgSizer2->Add( bSizer2, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	fgSizer2->Add( bSizer3, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer2 );
	this->Layout();

	m_pThread = NULL;
}

PasswordReminder::~PasswordReminder()
{
	safe_delete(m_pThread);
}

#ifdef NIX
bool PasswordReminder::Show(const bool show)
{
	bool ret = gcDialog::Show(show);
	if (ret)
		this->Raise();
	return ret;
}
#endif

bool PasswordReminder::isEmailValid()
{
	wxString str = m_tbEmail->GetValue();
	gcString email((const wchar_t*)str.c_str());

	return UTIL::MISC::isValidEmail(email.c_str());
}

void PasswordReminder::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butSend->GetId())
	{
		if (!isEmailValid())
		{
			gcMessageBox(this, Managers::GetString(L"#PR_VALIDEMAIL"), Managers::GetString(L"#PR_ERRTITLE"));
			return;
		}

		m_butSend->Enable(false);
		m_tbEmail->Enable(false);

		wxString str = m_tbEmail->GetValue();
		gcString email((const wchar_t*)str.c_str());

		m_pThread = new PassReminderThread(email.c_str());
		m_pThread->onCompleteEvent += guiDelegate(this, &PasswordReminder::onComplete);
		m_pThread->onErrorEvent += guiDelegate(this, &PasswordReminder::onError);
		m_pThread->start();
	}
	else
	{
		EndModal(0);
	}
}


void PasswordReminder::onTextBoxEnter( wxCommandEvent& event )
{
	wxCommandEvent newE;
	newE.SetId(m_butSend->GetId());
	onButtonClicked(newE);
}

void PasswordReminder::onComplete()
{
	gcMessageBox(this, Managers::GetString(L"#PR_SENT"), Managers::GetString(L"#PR_TITLE"));
	Close();
}

void PasswordReminder::onError(gcException& e)
{
	gcErrorBox(this, "#PR_ERRTITLE", "#PR_ERROR", e);
	Close();
}

