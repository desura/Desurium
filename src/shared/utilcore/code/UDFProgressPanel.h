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

#ifndef DESURA_UDFPROGRESSPANEL_H
#define DESURA_UDFPROGRESSPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/gauge.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>

#include "wx_controls/gcControls.h"
#include "wx_controls/gcSpinnerProgBar.h"

namespace UserCore
{
	class UserI;
}

class UninstallAllThread;

class UDFProgressPanel : public gcPanel 
{
public:
	UDFProgressPanel(wxWindow* parent, UserCore::UserI* user, uint32 flags);
	~UDFProgressPanel();

protected:
	wxStaticText* m_labTotal;
	wxStaticText* m_labCurrent;

	gcSpinnerProgBar* m_pbTotal;
	gcSpinnerProgBar* m_pbCurrent;

	wxTextCtrl* m_tbInfo;

	void onProgress(std::pair<uint32, uint32> &prog);
	void onLog(gcString &msg);
	void onComplete();

private:
	UninstallAllThread* m_pThread;
};

#endif //DESURA_UDFPROGRESSPANEL_H
