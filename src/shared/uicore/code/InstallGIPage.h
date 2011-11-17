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

#ifndef DESURA_INSTALLGIPAGE_H
#define DESURA_INSTALLGIPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx_controls/gcSpinnerProgBar.h"
#include "BaseInstallPage.h"


namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

///////////////////////////////////////////////////////////////////////////////
/// Class InstallGIPage
///////////////////////////////////////////////////////////////////////////////



class InstallGIPage : public BaseInstallPage 
{
public:
	InstallGIPage(wxWindow* parent);
	~InstallGIPage();

	void init();

protected:
	gcStaticText* m_labInfo;
	gcSpinnerProgBar* m_pbProgress;
	gcButton* m_butCancel;


	void onButtonPressed(wxCommandEvent& event);
	
	void onItemUpdate(UserCore::Item::ItemInfoI::ItemInfo_s& info);
	void onComplete(uint32& itemId);
	void onError(gcException& e);
	void onProgressUpdate(uint32& prog);

	uint8 validate();
	void checkRequirements();

};

}
}
}

#endif //DESURA_INSTALLGIPAGE_H
