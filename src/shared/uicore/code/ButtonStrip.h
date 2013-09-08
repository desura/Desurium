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

#ifndef DESURA_BUTTONSTRIP_H
#define DESURA_BUTTONSTRIP_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx_controls/gcImageHandle.h"



class ButtonStrip : public gcPanel
{
public:
	ButtonStrip(wxWindow *parent);
	~ButtonStrip();

protected:
	gcImgButtonCount *m_pThreadButton;
	gcImgButtonCount *m_pPMsgButton;
	gcImgButtonCount *m_pUpdateButton;
	gcImgButtonCount *m_pCartButton;

	void onButtonClicked(wxCommandEvent& event);
	void OnEraseBG( wxEraseEvent& event );

	void onUserUpdate();
private:
	gcImageHandle m_imgBg;

	DECLARE_EVENT_TABLE();
};




#endif //DESURA_BUTTONSTRIP_H
