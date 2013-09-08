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

#ifndef DESURA_CONSOLE_H
#define DESURA_CONSOLE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include <wx/richtext/richtextctrl.h>

#include "util_thread/BaseThread.h"
#include "Color.h"
#include "Managers.h"

typedef struct
{
	gcWString str;
	Color col;
} ConsoleText_s;


///////////////////////////////////////////////////////////////////////////////
/// Class Console
///////////////////////////////////////////////////////////////////////////////
class Console : public gcFrame 
{
public:
	Console(wxWindow* parent);
	~Console();

	WINDOW_UNREG();

	void appendText(gcWString text, Color col = Color(0));
	
	void processCommand();
	void conDump();
	void applyTheme();
	void setSize();
	void postShowEvent();

protected:
	void setupAutoComplete();

	wxRichTextCtrl* m_rtDisplay;
	gcComboBox* m_tbInfo;
	gcButton* m_butSubmit;
	
	void onWindowClose( wxCloseEvent& event );
	void onSubmitClicked( wxCommandEvent& event );
	void onKeyDown( wxKeyEvent& event );	

	void onShow(uint32&);
	void onConsoleText(ConsoleText_s& text);

	Event<ConsoleText_s> consoleTextEvent;
	Event<uint32> showEvent;

	wxBoxSizer* m_pSizer;

private:
	bool m_bCenterOnParent;
};

#endif //DESURA_LOG_FORM_H
