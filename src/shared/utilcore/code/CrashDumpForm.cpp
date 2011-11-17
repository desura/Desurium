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
#include "CrashDumpForm.h"

#include "wx_controls/gcControls.h"
#include "wx_controls/gcManagers.h"

#include "CDOverView.h"
#include "CDProcess.h"

CrashDumpForm::CrashDumpForm(wxWindow* parent) : gcFrame(parent, wxID_ANY, L"Desura: CrashDump Utility", wxDefaultPosition, wxSize(370, 210), wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CrashDumpForm::onButtonClicked, this); 
	Bind(wxEVT_CLOSE_WINDOW, &CrashDumpForm::onClose, this);

	SetTitle(Managers::GetString(L"#CRASH_TITLE"));

	Centre(wxBOTH);

	m_pPanel = new CDOverView(this);

	wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
	bSizer4->Add(m_pPanel, 1, wxEXPAND, 5);
	
	this->SetSizer(bSizer4);
	this->Layout();
}

CrashDumpForm::~CrashDumpForm()
{
}

void CrashDumpForm::onClose(wxCloseEvent &event)
{
	GetParent()->Close();
}

void CrashDumpForm::onButtonClicked(wxCommandEvent &event)
{
	if (event.GetId() == CD_CLOSE)
	{
		CDProcess* process = dynamic_cast<CDProcess*>(m_pPanel);

		if (process)
			process->stop();

		Close();
	}
	else 
	{
		m_pPanel->Show(false);
		m_pPanel->Destroy();

		CDProcess* process = new CDProcess(this);
		m_pPanel = process;

		wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
		bSizer4->Add(m_pPanel, 1, wxEXPAND, 5);
	
		this->SetSizer(bSizer4);
		this->Layout();

		if (event.GetId() == CD_GENERATE)
			process->start(true);
		else if (event.GetId() == CD_UPLOAD)
			process->start(false);
	}
}