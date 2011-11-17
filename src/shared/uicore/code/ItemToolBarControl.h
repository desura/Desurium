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

#ifndef DESURA_PAGEITEMCONTROL_H
#define DESURA_PAGEITEMCONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseToolBarControl.h"
#include "HtmlToolBarControl.h"

enum
{
	BUTTON_EXPAND = BUTTON_LAST,
	BUTTON_CONTRACT,
	BUTTON_GAME,
	BUTTON_FAV,
	BUTTON_TOOL,
	BUTTON_DEV,
};

class ItemButton;
class SearchControl;

class ItemToolBarControl: public BaseToolBarControl 
{
public:
	ItemToolBarControl(wxWindow* parent);
	~ItemToolBarControl();
	
	virtual void onActiveToggle(bool state);

	void focusSearch();

protected:
	void onItemsAdded(uint32&);
	void onUploadItemsAdded();

	void createButtons();
	void onLoginItemsLoaded();

private:
	wxFlexGridSizer* m_pFGContentSizer;

	SearchControl* m_pSearch;

	gcImageButton* m_butExpand;
	gcImageButton* m_butContract;

	std::vector<ItemButton*> m_vButtonList;
};

#endif //DESURA_PAGEITEMCONTROL_H
