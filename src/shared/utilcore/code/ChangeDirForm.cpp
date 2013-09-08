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
#include "ChangeDirForm.h"


ChangeDirForm::ChangeDirForm(wxWindow* parent, const char* dest, UserCore::UserI* user) : gcFrame(parent, wxID_ANY, "#CDF_TITLE", wxDefaultPosition, wxSize( 370,100 ), wxCAPTION|wxTAB_TRAVERSAL)
{
	m_pThread = NULL;
	m_uiLastStage = -1;

	SetTitle(Managers::GetString(L"#CDF_TITLE"));

	m_labInfo = new wxStaticText(this, wxID_ANY, Managers::GetString(L"#CDF_STARTUP"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinnerProgBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 22));


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( m_labInfo, 1, wxEXPAND, 5 );
	fgSizer1->Add( m_pbProgress, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );

	start(dest, user);
}

ChangeDirForm::~ChangeDirForm()
{
	if (m_pThread)
		m_pThread->stop();

	safe_delete(m_pThread);
}

void ChangeDirForm::start(const char* dest, UserCore::UserI* user)
{
	m_pThread = new ChangeDirThread(dest, user);

	m_pThread->onErrorEvent += guiDelegate(this, &ChangeDirForm::onError);
	m_pThread->onProgressEvent += guiDelegate(this, &ChangeDirForm::onProgress);
	m_pThread->onCompleteEvent += guiDelegate(this, &ChangeDirForm::onComplete);
	m_pThread->onStageEvent += guiDelegate(this, &ChangeDirForm::onStageChange);

	m_pThread->start();

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);
}

void ChangeDirForm::onError(gcException &e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(GetParent(), "#CDF_GERRTITLE", "#CDF_GERROR", e);
}

void ChangeDirForm::onStageChange(uint32 &stage)
{
	if (stage == 2)
	{
		m_labInfo->SetLabel(Managers::GetString(L"#CDF_MOVEFILE"));
	}
	else if (stage == 1)
	{
		m_labInfo->SetLabel(Managers::GetString(L"#CDF_DBCHANGES"));
	}
}

void ChangeDirForm::onProgress(uint32 &prog)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(prog);

	m_pbProgress->setProgress(prog);
}

void ChangeDirForm::onComplete()
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	restartDesura();
	GetParent()->Close();
}

void ChangeDirForm::restartDesura()
{
	char exePath[255];
	GetModuleFileNameA(NULL, exePath, 255);

	UTIL::FS::Path path = UTIL::FS::PathWithFile(exePath);

#ifdef DEBUG
	path += UTIL::FS::File("desura-d.exe");
#else
	path += UTIL::FS::File("desura.exe");
#endif

	PROCESS_INFORMATION ProcInfo = {0};
	STARTUPINFOA StartupInfo = {0};

	std::string curDir = path.getFullPath();

#ifdef DEBUG
	//launchArg += " -waitfordebugger";
#endif

	UTIL::WIN::launchExe(curDir.c_str());
}
