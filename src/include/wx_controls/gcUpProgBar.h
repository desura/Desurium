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

#ifndef DESURA_GCUPPROGBAR_H
#define DESURA__GCUPPROGBAR_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include <wx/control.h>

#include "gcProgressBar.h"

//! Progress bar for uploads that show an orange part for currently uploading section and blue bar for done
class gcULProgressBar : public gcProgressBar
{
public:
	gcULProgressBar(wxWindow *parent, int id = wxID_ANY );

	//! Sets the current percent to be the completed milestone
	void setMileStone();
	
	//! Reverts back to the last completed milestone
	void revertMileStone();

protected:
	virtual void doHandPaint(wxPaintDC& dc);
	virtual void doImgPaint(wxPaintDC& dc);

private:
	uint8 m_uiLastMS;
};

#endif
