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

#ifndef DESURA_GCFRAME_H
#define DESURA_GCFRAME_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/frame.h>
#include "guiDelegate.h"
#include "gcManagers.h"

#include "gcCustomFrame.h"

class CVarInfo;

//! A top level frame that uses desura theme and also can have gui events
class gcFrame : public wxGuiDelegateImplementation< gcCustomFrame<wxFrame> >
{
public:
	enum PROGSTATE
	{
		P_NONE,
		P_NORMAL,
		P_ERROR,
		P_PAUSED,
	};

	gcFrame();
	gcFrame(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, bool delayLoad = false);
	~gcFrame();

	WINDOW_UNREG();

	//! Sets the progress state in the taskbar for windows. Linux this is a noop
	//!
	void setProgressState(PROGSTATE state);
	
	//! Sets the progress percent in the taskbar for windows. Linux this is a noop
	//!	
	void setProgress(uint8 prog);

	//! Sets the size that this window should be and keeps resizing till it gets to this size. 
	//! This was to fix a bug where moving a window that tried to resize it self while moving would 
	//! cancel the resize
	//!
	void setIdealSize(int width, int height);

	//! Enables saving of window position in the CVar db
	//!
	void setupPositionSave(const char* name, bool saveMax = true, int defWidth = -1, int defHeight = -1);
	
	//! Starts the recording of windows position
	//!
	void enablePositionSave(bool state = true);
	
	//! Loads the last saved window position
	//!
	bool loadSavedWindowPos();

protected:
	void onIdle( wxIdleEvent& event );
	void initGCFrame(bool delayLoad, long style);

	void onMove(wxMoveEvent  &event);
	void onResize(wxSizeEvent &event);
	void onWindowClose(wxCloseEvent& event);

	CVar* getCVar(std::string name, std::string def);

	void onFormMax();
	EventV onFormMaxEvent;

private:
	wxSize m_IdealSize;
	bool m_bEnableIdealSize;

	CVarInfo* m_pCVarInfo;

	DECLARE_EVENT_TABLE();
};

#endif
