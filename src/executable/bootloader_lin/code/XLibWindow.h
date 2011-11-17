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

#ifndef DES_UPDATE_WINDOW_H
#define DES_UPDATE_WINDOW_H

#include <cstdio>
#include <cstring>
#include <string>

#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

class XlibWindow
{
public:
	XlibWindow();
	~XlibWindow();
	
	void RunEvents();
	void StopEvents();
	
	void Show();
	void Hide();
	void Destroy(bool CallOnDestroy = true);
	void Redraw();
	
	void SetCurrent(float currentBytes);
	void SetTotal(float totalBytes);
	void SetStatus(const char* status);
	void SetPercentage(float percentage = 0);
	
	void IncrementPercentage(float amount = 1);
	void HandleEvent(XEvent event);
	
	bool IsRunning();
	XEvent m_event;

protected:
	virtual void On_StopEvents() {};
	virtual void On_Show() {};
	virtual void On_Hide() {};
	virtual void On_RunEvents() {};
	virtual void On_Destroy() {};
	
	virtual void onCancel(){}
	
	static void* WrapThread(void* data);
	void RunThread();
	
private:
	Display *m_display;
	GC m_gc;
	
	Window m_window;
	Window m_progress_bar;
	
	int m_orig_x;
	int m_orig_y;	
	int m_event_mask;
	
	double m_dTotalBytes;
	double m_dCurrentBytes;	
	volatile float m_percentage;
	
	bool m_bLeftMouseDown;
	bool m_bCancelHover;	
	volatile bool m_run;
	
	pthread_t m_runthread;
	
	XTextProperty m_text_prop;
	XpmAttributes m_xpm_attr;
	XFontStruct* m_mainFont;
	
	Atom m_atom;
	std::string m_szStatus;
	std::string m_szOldStatus;
	
	Pixmap m_bg_pixmap;
	Pixmap m_bar_pixmap;

	XFontStruct* m_CurrFont;
};

#endif // DES_UPDATE_WINDOW_H
