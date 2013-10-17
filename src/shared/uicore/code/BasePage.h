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

#ifndef DESURA_INSTALLBASEPAGE_H
#define DESURA_INSTALLBASEPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

#define TYPE_FOLDER true
#define TYPE_FILE false

namespace UserCore
{
	namespace Item
	{
		class ItemInfoI;
	}
}


//! Base class that repersents a basic page on a multipage form
//!
class BasePage : public gcPanel 
{
public:
	BasePage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );

	//! Sets the item info id
	//!
	//! @param id Item id
	//!
	virtual void setInfo(DesuraId id);

	//! Sets the mcf build number
	//! Maybe this shouldnt be here?
	//!
	//! @param branch Mcf branch
	//! @param build Mcf Build
	//!
	virtual void setMCFInfo(MCFBranch branch, MCFBuild build);

	//! Sets the current page task to paused or not
	//!
	//! @param state Set paused or not
	//!
	virtual void setPaused(bool state=true){;}

	//! Gets the item info form the id number
	//!
	//! @return ItemInfo if found or NULL if not
	//!
	UserCore::Item::ItemInfoI* getItemInfo();

	//! Gets the item id number
	//!
	//! @return Item id
	//!
	DesuraId getItemId(){return m_iInternId;}


	//! Overide this to start the task needed when this page is active
	//!
	virtual void run()=0;

	//! Overide this to clean up once the page becomes inactive
	//!
	virtual void dispose()=0;

protected:
	DesuraId m_iInternId;
	uint32 m_uiMCFBuild;
	uint32 m_uiMCFBranch;
};


#endif //DESURA_INSTALLBASEPAGE_H
