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

#ifndef DESURA_BASEMENUBUTTON_H
#define DESURA_BASEMENUBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

enum SIDE
{
	RIGHT,
	LEFT,
};

class gcMainCustomFrameImpl;

class BaseMenuButton
{
public:
	BaseMenuButton(wxWindow* button);
	virtual ~BaseMenuButton();

	virtual void setSelected(bool state){m_bSelected = state;onSelected();}
	void setSelected(bool state, SIDE s){m_bRight = (s==RIGHT);setSelected(state);}
	
	bool isSelected(){return m_bSelected;}

	//this is for the selected menu border images
	bool isRight(){return m_bRight;}

	wxWindow* getButton(){return m_pButton;}

protected:
	virtual void onSelected(){;}

private:
	wxWindow* m_pButton;
	bool m_bSelected;
	bool m_bRight;
};

#endif //DESURA_BASEMENUBUTTON_H
