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
#include "UploadProgPage.h"
#include "UploadForm.h"

#include "MainApp.h"
#include "Managers.h"


///////////////////////////////////////////////////////////////////////////////
/// Class UploadProgPage
///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( UploadProgPage, BasePage )
	EVT_BUTTON( wxID_ANY, UploadProgPage::onButClick )
	EVT_CHECKBOX( wxID_ANY, UploadProgPage::onChecked )
END_EVENT_TABLE()

UploadProgPage::UploadProgPage(wxWindow* parent) 
	: BasePage(parent, wxID_ANY, wxDefaultPosition, wxSize( 400,100 ), wxTAB_TRAVERSAL)
	, m_llTotalUpload(0)
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 5, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 3 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#UDF_ETIME"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer5->Add( m_staticText3, 0, wxALIGN_BOTTOM|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_labTimeLeft = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#UDF_NOTSTARTED"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labTimeLeft->Wrap( -1 );
	bSizer5->Add( m_labTimeLeft, 0, wxALIGN_BOTTOM|wxTOP|wxRIGHT, 5 );
	

	m_pbProgress = new gcULProgressBar( this, wxID_ANY );


	m_cbDeleteMcf = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#UDF_DELETE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbDeleteMcf->SetValue( true ); 
	m_cbDeleteMcf->SetToolTip( Managers::GetString(L"#UDF_DELETE_TOOLTIP") );


	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_butPause = new gcButton( this, wxID_ANY, Managers::GetString(L"#PAUSE"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_butPause, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );
	


	fgSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
	fgSizer1->Add( m_pbProgress, 0, wxALL|wxEXPAND, 5 );
	fgSizer1->Add( m_cbDeleteMcf, 0, wxLEFT|wxEXPAND, 5);
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer4, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();

	m_bDone = false;

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);
}

UploadProgPage::~UploadProgPage()
{
	dispose();
}

void UploadProgPage::dispose()
{
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);

	if (info)
	{
		*info->getActionEvent() -= guiDelegate(this, &UploadProgPage::onAction);
		*info->getUploadProgressEvent() -= guiDelegate(this, &UploadProgPage::onProgress);
		*info->getErrorEvent() -= guiDelegate(this, &UploadProgPage::onError);
		*info->getCompleteEvent() -= guiDelegate(this, &UploadProgPage::onComplete);
	}
}

void UploadProgPage::setInfo(DesuraId id, uint32 hash, uint32 start)
{
	BasePage::setInfo(id);

	m_iStart = start;
	m_uiUploadHash = hash;
}

void UploadProgPage::run()
{
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);
	assert(info);

	*info->getActionEvent() += guiDelegate(this, &UploadProgPage::onAction);
	*info->getUploadProgressEvent() += guiDelegate(this, &UploadProgPage::onProgress);
	*info->getErrorEvent() += guiDelegate(this, &UploadProgPage::onError);
	*info->getCompleteEvent() += guiDelegate(this, &UploadProgPage::onComplete);

	info->setStart(m_iStart);

	if (info->isPaused())
		info->unpause();

	info->start();

	m_cbDeleteMcf->SetValue(info->shouldDelMcf());
}


void UploadProgPage::onButClick( wxCommandEvent& event )
{ 
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);
	assert(info);

	if (event.GetId() == m_butPause->GetId())
	{
		if (info->isPaused())
			info->unpause();
		else
			info->pause();
	}
	else if (event.GetId() == m_butCancel->GetId())
	{
		if (!info->isPaused())
		{
			GetUploadMng()->removeUpload(info->getKey());
			UTIL::FS::delFile(info->getFile());
		}

		UploadMCFForm* temp = dynamic_cast<UploadMCFForm*>(GetParent());

		if (temp)
			temp->setTrueClose();

		GetParent()->Close();
	}
	else
	{
		event.Skip(); 
	}
}

void UploadProgPage::onChecked(wxCommandEvent& event)
{
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);
	assert(info);

	info->setDelMcf(m_cbDeleteMcf->GetValue());
}

void UploadProgPage::onAction()
{
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());


	if (!info->isPaused())
	{
		m_butPause->SetLabel(Managers::GetString(L"#PAUSE"));
		m_butCancel->SetLabel(Managers::GetString(L"#CANCEL"));

		if (par)
			par->setProgressState(gcFrame::P_NORMAL);
	}
	else
	{
		m_butPause->SetLabel(Managers::GetString(L"#RESUME"));
		m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));

		if (par)
			par->setProgressState(gcFrame::P_PAUSED);
	}

	m_cbDeleteMcf->SetValue(info->shouldDelMcf());

	Refresh();
	m_cbDeleteMcf->Refresh();
}


void UploadProgPage::onComplete(uint32& status)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	UploadMCFForm* temp = dynamic_cast<UploadMCFForm*>(GetParent());

	if (temp)
		temp->setTrueClose();

	std::string done = UTIL::MISC::niceSizeStr(m_llTotalUpload, true);
	std::string total = UTIL::MISC::niceSizeStr(m_llTotalUpload);
	m_pbProgress->setCaption(gcString("{0} of {1}", done, total));
	m_pbProgress->setProgress(100);
	m_pbProgress->setMileStone();

	m_staticText3->SetLabel(Managers::GetString(L"#UDF_COMPLETE"));
	m_labTimeLeft->SetLabel(wxT(""));

	m_butPause->Enable(false);
	m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));
	m_bDone = true;

	this->Show();
}


void UploadProgPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(GetParent(), "#UDF_ERRTITLE", "#UDF_ERROR", e);
	UserCore::Misc::UploadInfoThreadI* info = GetUploadMng()->findItem(m_uiUploadHash);

	if (info)
		info->nonBlockStop();

	UploadMCFForm* temp = dynamic_cast<UploadMCFForm*>(GetParent());

	if (temp)
		temp->setTrueClose();

	GetParent()->Close();
}

void UploadProgPage::onProgress(UserCore::Misc::UploadInfo& info)
{
	if (info.milestone)
	{
		m_pbProgress->setMileStone();
		m_pbProgress->Update();

#ifdef NIX
		Refresh(false);
#endif
		return;
	}

	m_llTotalUpload = info.totalAmmount;

	std::string done = UTIL::MISC::niceSizeStr(info.doneAmmount, true);
	std::string total = UTIL::MISC::niceSizeStr(info.totalAmmount);
	m_pbProgress->setCaption(gcString("{0} of {1}", done, total));


	if (info.paused)
	{
		m_labTimeLeft->SetLabel(Managers::GetString(L"#UDF_UNKNOWNPAUSE"));
		m_pbProgress->revertMileStone();
		return;
	}

	std::string lab = UTIL::MISC::genTimeString(info.hour, info.min, info.rate);
	m_labTimeLeft->SetLabel(lab);

	m_pbProgress->setProgress(info.percent);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(info.percent);

#ifdef NIX
	Refresh(false);
#endif

	Update();
}
