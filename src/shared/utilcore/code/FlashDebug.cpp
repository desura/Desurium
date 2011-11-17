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

#include "Common.h"
#include "FlashDebug.h"


class RunThread : public Thread::BaseThread
{
public:
	RunThread(HWND hwnd) : Thread::BaseThread("FlashThread")
	{
		m_Hwnd = hwnd;
	}

	void run()
	{
		gcSleep(5000);

#ifdef WIN32
		FLASHWINFO info;

		memset(&info, 0, sizeof(FLASHWINFO));
		info.cbSize = sizeof(FLASHWINFO);
		info.hwnd = m_Hwnd;
		info.dwFlags = FLASHW_TRAY|FLASHW_TIMERNOFG;
		info.uCount = 20;

		FlashWindowEx(&info);
#endif

	}

	HWND m_Hwnd;
};


FlashDebug::FlashDebug(wxWindow* parent) : gcFrame(parent, wxID_ANY, "Flash Debug", wxDefaultPosition, wxSize( 700,468 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 2, 2, 0, 0 );
	
	wxButton* m_button1 = new wxButton( this, wxID_ANY, wxT("MyButton"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( m_button1, 0, wxALL, 5 );
	
	this->SetSizer( gSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &FlashDebug::onButtonClicked, this);
}

FlashDebug::~FlashDebug()
{

}

void FlashDebug::onButtonClicked(wxCommandEvent &event)
{
	RunThread* thread = new RunThread((HWND)this->GetHWND());
	thread->start();
}