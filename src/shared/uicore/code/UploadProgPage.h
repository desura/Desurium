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


#ifndef DESURA_UPLOADPROGPAGE_H
#define DESURA_UPLOADPROGPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"

#include "Event.h"
#include "usercore/UploadInfo.h"
#include "wx_controls/gcControls.h"

#include <wx/tglbtn.h>


///////////////////////////////////////////////////////////////////////////////
/// Class UploadProgPage
///////////////////////////////////////////////////////////////////////////////
class UploadProgPage : public BasePage 
{
public:
	UploadProgPage(wxWindow* parent);
	~UploadProgPage();

	void dispose();

	void setInfo(DesuraId id, uint32 hash, uint32 start);
	void run();

protected:
	void uploadDone();
	void onButClick( wxCommandEvent& event );

	void onUploadUpdate();
	void onComplete(uint32& status);
	void onError(gcException& e);
	void onProgress(UserCore::Misc::UploadInfo& info);
	void onAction();
	void onChecked( wxCommandEvent& event );

	gcStaticText* m_staticText3;
	gcStaticText* m_labTimeLeft;

	gcULProgressBar* m_pbProgress;
	
	gcButton* m_butPause;
	gcButton* m_butCancel;

	gcCheckBox *m_cbDeleteMcf;

private:	
	uint32 m_iStart;
	uint32 m_uiUploadHash;

	bool m_bDone;

	uint64 m_llTotalUpload;

	DECLARE_EVENT_TABLE();
};


#endif //DESURA_UPLOADPROGPAGE_H
