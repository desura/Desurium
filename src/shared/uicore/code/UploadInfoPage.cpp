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
#include "UploadInfoPage.h"

#include "MainApp.h"
#include "MainForm.h"
#include "Managers.h"

#include "UploadForm.h"


class fileValidateThread : public Thread::BaseThread
{
public:
	fileValidateThread(const char* file, const char* md5, uint32 size) : BaseThread("FileValidation Thread")
	{
		m_szFile = gcString(file);
		m_szMd5 = gcString(md5);
		m_uiSize = size;
	}

	Event<fiInfo> onCompleteEvent;

protected:
	void run()
	{
		bool res = false;

		if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(m_szFile)))
		{
			res = false;
		}
		else
		{
			std::string md5 = UTIL::MISC::hashFile(m_szFile.c_str());
			res = (UTIL::FS::getFileSize(UTIL::FS::PathWithFile(m_szFile)) == m_uiSize && md5 == m_szMd5);
		}

		fiInfo fi;
		fi.path = m_szFile.c_str();
		fi.res = res?1:0;

		onCompleteEvent(fi);
	}

private:
	gcString m_szFile;
	gcString m_szMd5;
	uint32 m_uiSize;
};


///////////////////////////////////////////////////////////////////////////////
/// UploadInfoPage
///////////////////////////////////////////////////////////////////////////////


UploadInfoPage::UploadInfoPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UploadInfoPage::onButtonClicked, this);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &UploadInfoPage::onTextChange, this);

	m_tbItemFile = NULL;

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	
	wxBoxSizer* bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_labText = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#UDF_PROMPT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labText->Wrap( -1 );
	bSizer5->Add( m_labText, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	

	wxBoxSizer* bSizer8  = new wxBoxSizer( wxHORIZONTAL );
	
	m_tbItemFile = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_tbItemFile, 1, wxBOTTOM|wxLEFT, 5 );
	
	m_butFile = new gcButton(this, wxID_ANY, Managers::GetString(L"#BROWSE"));
	bSizer8->Add( m_butFile, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	

	
	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_butUpload = new gcButton( this, wxID_ANY, Managers::GetString(L"#OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butUpload, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butCancel, 0, wxALL, 5 );
	


	fgSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer8, 1, wxEXPAND, 5 );
	fgSizer1->Add(0, 0, 1, wxEXPAND, 5);
	fgSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();
	
	m_pPrepThread = NULL;
	m_pUpInfo = NULL;

	m_pResumeThread = NULL;
	m_pVFThread = NULL;

	m_bResume = false;

	onResumeEvent += guiDelegate(this, &UploadInfoPage::onResume);
}

UploadInfoPage::~UploadInfoPage()
{
	dispose();
}

void UploadInfoPage::dispose()
{
	if (m_pVFThread)
		m_pVFThread->onCompleteEvent -= guiDelegate(this, &UploadInfoPage::onFileValidationComplete);

	safe_delete(m_pUpInfo);
	safe_delete(m_pVFThread);
	safe_delete(m_pPrepThread);
	safe_delete(m_pResumeThread);
	safe_delete(m_pUpInfo);
}


void UploadInfoPage::onTextChange( wxCommandEvent& event )
{
	validateInput();
}


void UploadInfoPage::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butUpload->GetId())
	{
		m_butUpload->Enable(false);
		m_tbItemFile->Enable(false);
		m_butFile->Enable(false);

		gcString path((const wchar_t*)m_tbItemFile->GetValue().c_str());
		initUpload(path.c_str());
	}
	else if (event.GetId() == m_butCancel->GetId())
	{
		GetParent()->Close();
	}
	else if (event.GetId() == m_butFile->GetId())
	{
		showDialog();
	}
}


void UploadInfoPage::showDialog()
{
	wxString curPath = m_tbItemFile->GetValue();

	wxFileDialog *p = new wxFileDialog(this, wxT("Select a MCF file"), curPath, wxT(""),  wxT("*.mcf"), wxFLP_FILE_MUST_EXIST|wxFLP_OPEN );
	if (p->ShowModal() == wxID_OK)
	{
		m_tbItemFile->SetValue(p->GetPath());
	}

	wxDELETE(p);
}

bool UploadInfoPage::validateInput()
{
	if (!m_tbItemFile)
		return false;

	bool state = true;

	if (!validatePath(m_tbItemFile, TYPE_FILE))
		state = false;

	m_butUpload->Enable( state );

	return state;
};

bool UploadInfoPage::validatePath(wxTextCtrl* ctrl, bool type)
{
	bool doesExsist = false;

	gcString dir((const wchar_t*)ctrl->GetValue().c_str());

	if (type == TYPE_FOLDER)
		doesExsist = UTIL::FS::isValidFolder(UTIL::FS::PathWithFile(dir));
	else
		doesExsist = UTIL::FS::isValidFile(UTIL::FS::PathWithFile(dir));

	if (doesExsist)
		ctrl->SetForegroundColour( *wxBLACK );
	else
		ctrl->SetForegroundColour( *wxRED );


	ctrl->Refresh();
	return doesExsist;
}

void UploadInfoPage::resetAllValues()
{
	//m_dpFile->SetPath(wxT(""));
	m_butUpload->Enable( false );

	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(getItemId());

	gcString filePath;
	gcString cachePath = GetUserCore()->getMcfCachePath();

	if (!item)
		filePath = gcString("{0}{1}temp{1}", cachePath, DIRS_STR);
	else
		filePath = gcString("{0}{2}{1}", cachePath, item->getId().getFolderPathExtension().c_str(), DIRS_STR);

	gcWString wfilePath(filePath);
	m_tbItemFile->SetValue(wfilePath.c_str());
}

void UploadInfoPage::setInfo(DesuraId id)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item && !GetUserCore()->isAdmin())
	{	
		Close();
		return;
	}

	BasePage::setInfo(id);
	resetAllValues();
}

void UploadInfoPage::setInfo_key(DesuraId id, const char* key)
{
	setInfo(id);

	if (key)
	{
		m_szKey = gcString(key);
		onResumeEvent();
	}
}

void UploadInfoPage::setInfo_path(DesuraId id, const char* path)
{
	setInfo(id);
	if (path)
	{
		m_tbItemFile->SetValue(gcString(path));

		if (validateInput())
		{
			m_butUpload->Enable(false);
			m_tbItemFile->Enable(false);
			m_butFile->Enable(false);

			initUpload(path);
		}
		else
		{
			gcMessageBox(GetParent(),  Managers::GetString(L"#UDF_ERROR_VALIDATION"), Managers::GetString(L"#UDF_ERRTITLE") );
		}
	}
}

void UploadInfoPage::onResume()
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(getItemId());

	if (!item && !GetUserCore()->isAdmin())
	{	
		Close();
		return;
	}

	m_tbItemFile->SetLabel( Managers::GetString(L"#UDF_RESUMEDIR"));

	m_butUpload->Enable(false);
	m_tbItemFile->Enable(false);
	m_butFile->Enable(false);

	m_bResume = true;

	safe_delete(m_pUpInfo);
	m_pUpInfo = new WebCore::Misc::ResumeUploadInfo();

	m_pResumeThread = GetThreadManager()->newUploadResumeThread(getItemId(), m_szKey.c_str(), m_pUpInfo);

	*m_pResumeThread->getErrorEvent() += guiDelegate(this, &UploadInfoPage::onError);
	*m_pResumeThread->getCompleteStringEvent() += guiDelegate(this, &UploadInfoPage::onResumeCompleteCB);

	m_pResumeThread->start();

	Show();
	Raise();
}

void UploadInfoPage::onResumeComplete(const char* path)
{
	if (path && strcmp(path, "NULL") != 0)
	{
		initUpload(path, m_pUpInfo->upsize);
	}
	else
	{
		m_butUpload->Enable(true);
		m_tbItemFile->Enable(true);
		m_butFile->Enable(true);

		resetAllValues();

		gcMessageBox(GetParent(),  Managers::GetString(L"#UDF_NOFILE"), Managers::GetString(L"#UDF_RESUME_ERRTITLE") );
	}
}



//we use a thread here as md5 checks on large files take a while
void UploadInfoPage::fileValidation(const char* path)
{
	if (m_pVFThread && m_pVFThread->isRunning())
		return;

	safe_delete(m_pVFThread);

	m_pVFThread = new fileValidateThread(path, m_pUpInfo->szHash.c_str(), m_pUpInfo->size);
	m_pVFThread->onCompleteEvent += guiDelegate(this, &UploadInfoPage::onFileValidationComplete);
	m_pVFThread->start();
}


void UploadInfoPage::onFileValidate(bool res, const char* path)
{
	if (res)
	{
		initUpload(path, m_pUpInfo->upsize);
	}
	else
	{
		gcMessageBox(GetParent(),  Managers::GetString(L"#UDF_NONMATCHING_FILE"), Managers::GetString(L"#UDF_UPLOAD_ERRTITLE") );
		m_butUpload->Enable(true);
		m_tbItemFile->Enable(true);
		m_butFile->Enable(true);
	}
}

void UploadInfoPage::initUpload(const char* path, uint64 start)
{
	Show(false);

	UploadMCFForm* temp = dynamic_cast<UploadMCFForm*>(GetParent());

	uint32 hash = GetUploadMng()->addUpload(getItemId(), m_szKey.c_str(), path);

	if (temp)
		temp->showProg(hash, start);
}

void UploadInfoPage::initUpload(const char* path)
{
	if (m_pUpInfo)
	{
		fileValidation(path);
	}
	else
	{
		m_pPrepThread = GetThreadManager()->newUploadPrepThread(getItemId(), path);

		*m_pPrepThread->getErrorEvent() += guiDelegate(this, &UploadInfoPage::onError);
		*m_pPrepThread->getCompleteStringEvent() += guiDelegate(this, &UploadInfoPage::onComplete);

		m_pPrepThread->start();
	}
}



void UploadInfoPage::onResumeCompleteCB(gcString& file)
{
	onResumeComplete(file.c_str());
}

void UploadInfoPage::onComplete(gcString& key)
{
	gcString path((const wchar_t*)m_tbItemFile->GetValue().c_str());
	m_szKey = key;
	initUpload(path.c_str(), 0);
}

void UploadInfoPage::onFileValidationComplete(fiInfo& info)
{
	onFileValidate( info.res?true:false, info.path);
}


void UploadInfoPage::onError(gcException& e)
{
	gcErrorBox(GetParent(), "#UDF_ERRTITLE", "#UDF_ERROR", e);

	if (e.getErrId() == ERR_COMPLETED)
		GetUploadMng()->removeUpload(m_szKey.c_str(), true);

	GetParent()->Close();
}
