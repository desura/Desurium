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

#include "Common.h"
#include "CDProcess.h"

#include "CDOverView.h"
#include "CrashDumpThread.h"
#include "CrashDumpUploadThread.h"
	
CDProcess::CDProcess(wxWindow* parent) : gcPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
	setParentSize(370, 130);

	m_labStatus = new wxStaticText(this, wxID_ANY, Managers::GetString(L"#CRASH_UPLOADINFO"));
	m_pbProgress = new gcProgressBar(this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ));
	m_butClose = new gcButton(this, CD_CLOSE, Managers::GetString(L"#CANCEL"));


	wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer3->Add( m_butClose, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer6->Add( 0, 5, 1, wxEXPAND, 5 );
	fgSizer6->Add(m_labStatus, 0, wxLEFT|wxTOP, 5 );
	fgSizer6->Add(m_pbProgress, 0, wxEXPAND|wxALL, 5);
	

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer5->AddGrowableCol( 0 );
	fgSizer5->AddGrowableRow( 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer5->Add( fgSizer6, 1, wxEXPAND, 5 );
	fgSizer5->Add( bSizer3, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer5 );
	this->Layout();

	m_pThread = NULL;
}

CDProcess::~CDProcess()
{
	safe_delete(m_pThread);
}

void CDProcess::start(bool generateDump)
{
	if (generateDump)
	{
		m_pThread = new CrashDumpThread();
		m_labStatus->SetLabel(Managers::GetString(L"#CRASH_GENERATEINFO"));
	}
	else
	{
		m_pThread = new CrashDumpUploadThread();
		m_labStatus->SetLabel(Managers::GetString(L"#CRASH_UPLOADINFO"));
	}

	m_pThread->onErrorEvent += guiDelegate(this, &CDProcess::onError);
	m_pThread->onCompleteEvent += guiDelegate(this, &CDProcess::onComplete);
	m_pThread->onProgressEvent += guiDelegate(this, &CDProcess::onProgress);

	m_pThread->start();
}

void CDProcess::stop()
{
	m_pThread->stop();
}

void CDProcess::onError(gcException& e)
{
	gcErrorBox(this, Managers::GetString("#CRASH_TITLE"), Managers::GetString("#CRASH_ERROR"), e);

	GetParent()->Show(false);
	GetParent()->Close();
}

void CDProcess::onComplete()
{
	GetParent()->Close();
}

void CDProcess::onProgress(uint8 &prog)
{
	m_pbProgress->setProgress(prog);
}