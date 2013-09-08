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
#include "DesuraServiceError.h"
#include "MainApp.h"

class DSEThread : public Thread::BaseThread
{
public:
	DSEThread(DesuraServiceError* caller) : BaseThread( "DSE Thread")
	{
		m_pCaller = caller;
	}

	~DSEThread()
	{
		stop();
	}

protected:
	void run()
	{
		if (!GetUserCore())
			return;

		try
		{
			GetUserCore()->restartPipe();
			m_pCaller->onFinishEvent();
		}
		catch (gcException &except)
		{
			m_pCaller->onErrorEvent(except);
		}
	}

private:
	DesuraServiceError* m_pCaller;
};

BEGIN_EVENT_TABLE( DesuraServiceError, gcDialog )
	EVT_BUTTON( wxID_ANY, DesuraServiceError::onButtonClicked )
END_EVENT_TABLE()

DesuraServiceError::DesuraServiceError( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#DS_ERROR"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap(360);
	fgSizer1->Add( m_labInfo, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_butReconnect = new gcButton( this, wxID_ANY, wxT("Reconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butReconnect, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butLogout = new gcButton( this, wxID_ANY, wxT("Logout"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butLogout, 0, wxALL, 5 );
	
	fgSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();

	m_pThread = NULL;

	onFinishEvent += guiDelegate(this, &DesuraServiceError::onFinish);
	onErrorEvent += guiDelegate(this, &DesuraServiceError::onError);
}

DesuraServiceError::~DesuraServiceError()
{
	safe_delete(m_pThread);
}

int DesuraServiceError::ShowModal()
{
	run();
	return gcDialog::ShowModal();
}

void DesuraServiceError::onFinish()
{
	EndModal(0);
}

void DesuraServiceError::onError(gcException& e)
{
	m_labInfo->SetLabel(gcString("{0} {1}\n", Managers::GetString("#DS_ERRORFAILED"), e));
	m_labInfo->Wrap(360);

	m_butReconnect->Enable(true);
	m_butLogout->Enable(true);
}

void DesuraServiceError::run()
{
	m_butReconnect->Enable(false);
	m_butLogout->Enable(false);

	safe_delete(m_pThread);

	m_pThread = new DSEThread(this);
	m_pThread->start();
}

void DesuraServiceError::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butReconnect->GetId())
	{
		run();
	}
	else if (event.GetId() == m_butLogout->GetId())
	{
		g_pMainApp->logOut();
		EndModal(0);
	}
}
