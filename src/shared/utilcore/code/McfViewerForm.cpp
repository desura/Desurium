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
#include "McfViewerForm.h"
#include "wx/filepicker.h"

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFHeaderI.h"
#include "mcfcore/MCFFileI.h"

#include "../../../branding/branding.h"

McfViewerForm::McfViewerForm(wxWindow* parent, gcString mcf) : gcFrame(parent, wxID_ANY, wxT(PRODUCT_NAME ": Mcf Viewer"), wxDefaultPosition, wxSize(370,500), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &McfViewerForm::onButtonClicked, this); 
	Bind(wxEVT_CLOSE_WINDOW, &McfViewerForm::onClose, this);

	this->SetSizeHints( wxSize( 370,400 ), wxDefaultSize );
	
	wxStaticText* m_staticText26 = new wxStaticText( this, wxID_ANY, wxT("Item Id"), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText27 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labId = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("File Version"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	wxStaticText* m_staticText15 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labVersion = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText23 = new wxStaticText( this, wxID_ANY, wxT("Type"), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText24 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labType = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("Build"), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText17 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labMCFVers = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText18 = new wxStaticText( this, wxID_ANY, wxT("Flags"), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText19 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labPatch = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText20 = new wxStaticText( this, wxID_ANY, wxT("Branch"), wxDefaultPosition, wxDefaultSize, 0 );
	wxStaticText* m_staticText21 = new wxStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labBranch = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );

	m_tcFileTree = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTR_DEFAULT_STYLE );

	m_butOpen = new gcButton(this, wxID_ANY, L"Open", wxDefaultPosition, wxSize(75,22));

	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 20, 3, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer11->Add( m_staticText26, 0, 0, 5 );
	fgSizer11->Add( m_staticText27, 0, 0, 5 );
	fgSizer11->Add( m_labId, 0, wxLEFT, 5 );
	
	fgSizer11->Add( m_staticText14, 0, 0, 5 );
	fgSizer11->Add( m_staticText15, 0, 0, 5 );
	fgSizer11->Add( m_labVersion, 0, wxLEFT, 5 );
	
	fgSizer11->Add( m_staticText23, 0, 0, 5 );
	fgSizer11->Add( m_staticText24, 0, 0, 5 );
	fgSizer11->Add( m_labType, 0, wxLEFT, 5 );
	
	fgSizer11->Add( m_staticText16, 0, 0, 5 );
	fgSizer11->Add( m_staticText17, 0, 0, 5 );
	fgSizer11->Add( m_labMCFVers, 0, wxLEFT, 5 );
	
	fgSizer11->Add( m_staticText18, 0, 0, 5 );
	fgSizer11->Add( m_staticText19, 0, 0, 5 );
	fgSizer11->Add( m_labPatch, 0, wxLEFT, 5 );
	
	fgSizer11->Add( m_staticText20, 0, 0, 5 );
	fgSizer11->Add( m_staticText21, 0, 0, 5 );
	fgSizer11->Add( m_labBranch, 0, wxLEFT, 5 );


	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer12->AddGrowableCol( 0 );
	fgSizer12->AddGrowableRow( 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	fgSizer12->Add( fgSizer11, 1, wxEXPAND, 7 );
	fgSizer12->Add( m_butOpen, 1, 0, 5 );

	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer10->AddGrowableCol( 0 );
	fgSizer10->AddGrowableRow( 1 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	fgSizer10->Add( fgSizer12, 1, wxEXPAND|wxALL, 7 );
	fgSizer10->Add( m_tcFileTree, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer10 );
	this->Layout();

	Centre(wxBOTH);

	if (mcf.size() > 0)
		loadMcf(mcf.c_str());
}

McfViewerForm::~McfViewerForm()
{
}

void McfViewerForm::onClose(wxCloseEvent &event)
{
	GetParent()->Close();
}

void McfViewerForm::onButtonClicked(wxCommandEvent& event)
{
	wxFileDialog temp(this, wxT("Select a MCF File"), wxEmptyString, wxEmptyString, wxT("*.mcf"), wxFLP_FILE_MUST_EXIST|wxFLP_OPEN, wxDefaultPosition );
	temp.ShowModal();

	wxString path = temp.GetPath();
	
	if (!path.empty())
	{
		loadMcf(path.c_str());
	}
}

void McfViewerForm::loadMcf(const char* path)
{
	m_pHandle = McfHandle();
	m_pHandle->setFile(path);
	m_pHandle->parseMCF();

	MCFCore::MCFHeaderI* temp = m_pHandle->getHeader();

	m_labId->SetLabel(gcWString(L"{0}", temp->getId()));
	m_labVersion->SetLabel(gcWString(L"{0}", temp->getFileVer()));
	m_labType->SetLabel(gcWString(L"{0}", temp->getType()));
	m_labMCFVers->SetLabel(gcWString(L"{0}", temp->getBuild().operator size_t()));
	m_labPatch->SetLabel(gcWString(L"{0}", temp->getFlags()));
	m_labBranch->SetLabel(gcWString(L"{0}", temp->getBranch().operator size_t()));

	//m_pHandle->Par
	uint32 count = m_pHandle->getFileCount();
	const char* mcfPath = m_pHandle->getFile();

	m_tcFileTree->DeleteAllItems();
	wxTreeItemId root = m_tcFileTree->AddRoot(UTIL::FS::PathWithFile(mcfPath).getFolderPath());
	m_tcFileTree->SetItemTextColour(root, *wxBLACK);



	for (uint32 x=0; x<count; x++)
	{
		MCFCore::MCFFileI* m_pHandleFile = m_pHandle->getMCFFile(x);
		
		if (!m_pHandleFile)
			continue;

		const char* path = m_pHandleFile->getPath();

		wxTreeItemId node = root;

		if (path && strcmp(path,"")!=0 && strcmp(path, "\\") !=0)
		{
			wxTreeItemId lastNode;
			wxTreeItemId item;
			
			gcString path(m_pHandleFile->getPath());
			gcWString wpath(path);

			node = recCreateNodes(wpath.c_str(), wpath.size(), root);
		}

		wxTreeItemId item = m_tcFileTree->AppendItem(node, m_pHandleFile->getName());

		if (!m_pHandleFile->isSaved())
			m_tcFileTree->SetItemTextColour(item, *wxRED);
		else
			m_tcFileTree->SetItemTextColour(item, *wxBLACK);

		// AddRoot
	}

	m_tcFileTree->Toggle(root);
}

wxTreeItemId McfViewerForm::recCreateNodes(const wchar_t* path, size_t pathSize, wxTreeItemId lastNode, bool search)
{
	if (!path)
		return lastNode;

	const wchar_t *nextPath = NULL;

	size_t x=0;
		
	if (path[0] == '\\' || path[0] == '/')
		path++;

	for (x; x<Safe::wcslen(path, pathSize); x++)
	{
		if (path[x] == '\\' || path[x] == '/')
		{
			nextPath = path+x+1;
			break;
		}
	}

	if (x==0)
		return lastNode;

	wchar_t *curFolder = new wchar_t[x+1];
	wcsncpy(curFolder, path, x);
	curFolder[x] = '\0';
	
	wxTreeItemId item;

	if (search)
		item = FindNode(curFolder, lastNode);
	
	if (!search || !item.IsOk())
	{
		lastNode = m_tcFileTree->AppendItem(lastNode, curFolder);
		m_tcFileTree->SetItemTextColour(lastNode, *wxBLACK);
		search = false;
	}
	else
	{
		lastNode = item;
	}

	safe_delete(curFolder);

	return recCreateNodes(nextPath, pathSize, lastNode, search);
}

wxTreeItemId McfViewerForm::FindNode(const wchar_t* str, wxTreeItemId lastNode)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId item = m_tcFileTree->GetFirstChild(lastNode, cookie);

	while (item.IsOk())
	{
		if (wcscmp(str, m_tcFileTree->GetItemText(item))==0)
			return item;

		item = m_tcFileTree->GetNextChild(lastNode, cookie);
	}

	return wxTreeItemId();
}
