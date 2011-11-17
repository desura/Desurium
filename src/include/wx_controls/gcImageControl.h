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

#ifndef DESURA_GCIMAGECONTROL_H
#define DESURA_GCIMAGECONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "gcPanel.h"
#include "gcImageHandle.h"
#include "gcFlickerFreePaint.h"

//! A image panel that can scale the image and also paint it with out flicker
class gcImageControl : public gcFlickerFreePaint<gcPanel>
{
public:
	gcImageControl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxFULL_REPAINT_ON_RESIZE );
	~gcImageControl();

	//! Sets the image path
	//!
	virtual void setImage(const char* image);

	//! Display the image in this propotion
	//!
	void setPropotional(uint32 x, uint32 y);
	
	//! Set if the background should be cleared before painting
	//!
	void setClearBG(bool state){m_bClearBG = state;}
	
	//! Set if the image should be tiled if it is too small
	//!
	void setTile(bool state){m_bTile = state;}
	
	//! Set if the image should be displayed with transperency enabled
	//!
	void setTrans(bool state){m_bTransEnabled = state;}

protected:
	virtual void doPaint(wxDC* dc);

	gcImageHandle m_imgHandle;

	bool m_bClearBG;
	bool m_bTile;
	bool m_bTransEnabled;

	bool m_bStoreRegion;
	wxRegion m_Region;

	uint32 m_uiXPro;
	uint32 m_uiYPro;
};

bool SetShape(const wxRegion& region, wxWindow* frame);

#endif
