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

#ifndef DESURA_GCSPINNERPROGBAR_H
#define DESURA_GCSPINNERPROGBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "gcSpinningBar.h"
#include "gcProgressBar.h"

//! A combination of a spinner bar and progress bar. When progress == 0 it spins other wise it shows progress.
class gcSpinnerProgBar : public gcPanel
{
public:
	gcSpinnerProgBar(wxWindow* parent, wxWindowID id = wxID_ANY,const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

	//! Force a swap between spinner and progress
	//!
	void swap();

	void setProgress(uint8 prog);
	uint8 getProgress();
	
	//! Set the text that shows in the middle of the progress bar
	//!
	void setCaption(const gcString& string);

protected:
	gcSpinningBar *m_sbSpinner;
	gcProgressBar *m_pbProgress;

	wxBoxSizer *m_pProgSizer;
	bool m_bSwapped;
};


#endif //DESURA_GCSPINNERPROGBAR_H
