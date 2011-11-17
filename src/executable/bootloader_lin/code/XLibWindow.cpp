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
#include "XLibWindow.h"

#include "background.xpm"
#include "bar.xpm"

XlibWindow::XlibWindow()
{
	ERROR_OUTPUT(__func__);
	m_run = false;
	m_percentage = 0;
	m_progress_bar = 0;
	m_bLeftMouseDown = false;
	m_bCancelHover = false;
	m_szStatus = "Initialising";
	m_dCurrentBytes = 0;
	m_dTotalBytes = 0;

	m_event_mask = ExposureMask | 
		ButtonPressMask | 
		ButtonReleaseMask |
		EnterWindowMask | 
		LeaveWindowMask |
		PointerMotionMask |
		FocusChangeMask |
		KeyPressMask |
		KeyReleaseMask |
		SubstructureNotifyMask |
		StructureNotifyMask |
		SubstructureRedirectMask;

	XInitThreads();
	
	m_display = XOpenDisplay(0);
	if (!m_display)
		return;

	m_gc = DefaultGC(m_display, DefaultScreen(m_display));
	m_window = XCreateWindow(m_display, DefaultRootWindow(m_display), 0, 0, 414, 218, 0, CopyFromParent, CopyFromParent, CopyFromParent, 0, 0);
	if (!m_window)
	{
		printf("Failed to create window!");
	}

	m_mainFont = XLoadQueryFont(m_display, "-monotype-arial-medium-r-normal-*-*-90-*-*-*-*-*-*");
		
	if (!m_mainFont)
		m_mainFont = XLoadQueryFont(m_display, "-adobe-helvetica-medium-r-normal-*-*-90-*-*-*-*-*-*");

	if (!m_mainFont)
		m_mainFont = XLoadQueryFont(m_display, "-dejavu-dejavu sans-medium-r-normal-*-*-90-*-*-*-*-*-*");
	
	if (!m_mainFont)
		m_mainFont = XLoadQueryFont(m_display, "-misc-fixed-medium-r-normal-*-*-90-*-*-*-*-*-*");
		
	if (!m_mainFont)
		ERROR_OUTPUT("Couldn't load font!");
		
	if (m_mainFont)
	{
		XSetFont(m_display, m_gc, m_mainFont->fid);	
		XFreeFont(m_display, m_mainFont);
	}
	
	XSetLineAttributes(m_display, m_gc, 1, LineSolid, CapNotLast, JoinMiter);


	m_text_prop.value = (unsigned char*)"Desura Updater";
	m_text_prop.encoding = XA_STRING;
	m_text_prop.format = 8;
	m_text_prop.nitems = 17;
	XSetWMName(m_display, m_window, &m_text_prop);

	m_xpm_attr.valuemask = XpmCloseness;
	m_xpm_attr.closeness = 40000;

	if(XpmCreatePixmapFromData(m_display, m_window, (char**)background_array, &m_bg_pixmap, NULL, &m_xpm_attr) == 0)
	{
		XSetWindowBackgroundPixmap(m_display, m_window, m_bg_pixmap);
	}
	else
	{
		fprintf(stderr, "Error: Could not load background pixmap!\n");
	}

	m_progress_bar = XCreateWindow(m_display, m_window, 38, 108, 1, 12, 0, CopyFromParent, CopyFromParent, CopyFromParent, 0, 0);
	if(m_progress_bar)
	{
		if(XpmCreatePixmapFromData(m_display, m_window, (char**)bar_array, &m_bar_pixmap, NULL, &m_xpm_attr) == 0)
		{
			XSetWindowBackgroundPixmap(m_display, m_progress_bar, m_bar_pixmap);
		}
		else
		{
			fprintf(stderr, "Error: Could not load bar pixmap!\n");
		}
	}

	XSizeHints* winHint = XAllocSizeHints();
	winHint->flags = PAllHints;
	winHint->max_width = 414;
	winHint->min_width = 414;
	winHint->max_height = 218;
	winHint->min_height = 218;
	XSetWMNormalHints(m_display, m_window, winHint);

	m_atom = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(m_display, m_window, &m_atom, 1);
	fflush(stdout);

	m_CurrFont = XQueryFont(m_display, XGContextFromGC(m_gc));
}

XlibWindow::~XlibWindow()
{
	ERROR_OUTPUT(__func__);
	Destroy();
}

void XlibWindow::RunEvents()
{
	ERROR_OUTPUT(__func__);

	m_run = true;

	if (pthread_create(&m_runthread, NULL, &XlibWindow::WrapThread, this) != 0)
		fprintf(stderr, "Error: Something went wrong spawning run thread!\n");

	On_RunEvents();
}

void XlibWindow::StopEvents()
{
	ERROR_OUTPUT(__func__);
	m_run = false;

	pthread_join(m_runthread, NULL);

	On_StopEvents();
}

void XlibWindow::Show()
{
	ERROR_OUTPUT(__func__);
	XSelectInput(m_display, m_window, m_event_mask);

	if (m_window)
		XMapWindow(m_display, m_window);

	if (m_progress_bar)
		XMapWindow(m_display, m_progress_bar);

	XMapSubwindows(m_display, m_window);

	if (! m_run)
		RunEvents();

	On_Show();
}

void XlibWindow::Hide()
{
	ERROR_OUTPUT(__func__);
	XUnmapWindow(m_display, m_window);
	On_Hide();
}

void XlibWindow::Destroy(bool CallOnDestroy)
{
	ERROR_OUTPUT(__func__);
	if (m_run == false)
		return;

	m_run = false;
	pthread_join(m_runthread, NULL);

	XCloseDisplay(m_display);
	
	if(CallOnDestroy)
		On_Destroy();
}

void XlibWindow::HandleEvent(XEvent event)
{
//	ERROR_OUTPUT(__func__);
	switch (event.type)
	{
		case ClientMessage:
		{
			Atom deleteAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
			if (deleteAtom == (Atom)event.xclient.data.l[0])
			{
				onCancel();
				Destroy();	
			}
			break;
		}
		case ButtonPress:
		{

			if (event.xbutton.button & Button1)
			{
				m_orig_x = event.xbutton.x;
				m_orig_y = event.xbutton.y;
				m_bLeftMouseDown = true;
			}

			break;
		}
		case ButtonRelease:
		{
			if ((event.xbutton.button & Button1) && (event.xbutton.x > 306 && event.xbutton.x < 382 && event.xbutton.y > 170 && event.xbutton.y < 192))
			{
				onCancel();
				Destroy();
			}

			m_bLeftMouseDown = false;
			break;
		}
		case MotionNotify:
		{
			if (m_bLeftMouseDown)
			{
				Window w, r = DefaultRootWindow(m_display);
				int wx, wy, rx, ry;
				unsigned m;
				XQueryPointer(m_display, m_window, &r, &w, &rx, &ry, &wx, &wy, &m);
				XMoveWindow(m_display, m_window, (rx - m_orig_x - 8), (ry - m_orig_y - 30));
				break;
			}

			if(event.xbutton.x > 306 && event.xbutton.x < 382 && event.xbutton.y > 170 && event.xbutton.y < 192)
			{
				m_bCancelHover = true;
			}
			else
			{
				m_bCancelHover = false;
			}
				
			break;
		}
		case Expose:
		{
			break;
		}
	}
}

void XlibWindow::Redraw()
{
//	ERROR_OUTPUT(__func__);
	// If we're not running, return
	if (!m_run)
		return;

	// Lock this display
	XLockDisplay(m_display);

	// Resize the progress bar window to the percentage
	if (m_percentage > 0.3)
	{
		XResizeWindow(m_display, m_progress_bar, ((342 * m_percentage) / 100), 12);
	}
	else if(m_percentage > 100)
	{
		XResizeWindow(m_display, m_progress_bar, 342, 12);
	}
	else
	{
		XResizeWindow(m_display, m_progress_bar, 1, 12);
	}

	// Setup the percent string
	char progress[255] = {0};
	if (strncmp(m_szStatus.c_str(), "Initialising", 12) == 0)
	{
		progress[0] = 0;
	}
 	else if (strncmp(m_szStatus.c_str(), "Downloading", 11) == 0)
	{
		if (m_dTotalBytes == 0)
		{
			snprintf(progress, 255, "%0d%%", (int)m_percentage);
		}
		else
		{
			snprintf(progress, 255, "%.2f/%.2f MiB", (m_dCurrentBytes / 1024 / 1024), (m_dTotalBytes / 1024 / 1024));
		}	
	}
	else if (strncmp(m_szStatus.c_str(), "Installing", 10) == 0)
	{
		snprintf(progress, 255, "%0d%%", (int)m_percentage);
	}
	else // Who knows
	{
		progress[0] = '\0';
	}

	// Get string widths
	int stringWidth = XTextWidth(m_CurrFont, m_szStatus.c_str(), m_szStatus.length());
	int statusWidth = XTextWidth(m_CurrFont, progress, strlen(progress));
	int cancelWidth = XTextWidth(m_CurrFont, "Cancel", strlen("Cancel"));
	
	// Set the standard grey
	XSetForeground(m_display, m_gc, 0x676767);

	// Draw the button borders and the cancel button label (no clearing needed)
	// -- straight over the old
	XDrawLine(m_display, m_window, m_gc, 308, 171, 381, 171);
	XDrawLine(m_display, m_window, m_gc, 381, 172, 381, 191);
	XDrawLine(m_display, m_window, m_gc, 381, 191, 308, 191);
	XDrawLine(m_display, m_window, m_gc, 307, 191, 307, 172);
		
	// If we're hovering over the cancel button, set foreground to blue-ish
	if (m_bCancelHover)
	{
		XSetForeground(m_display, m_gc, 0x218AF4);
	}
	
	// Dobn't have to clear this as it goes straight over the top
	XDrawString(m_display, m_window, m_gc, 303 + (cancelWidth / 2), 186, "Cancel", strlen("Cancel"));
	
	// Set the standard grey
	XSetForeground(m_display, m_gc, 0x676767);

	// Clear the window and flush the display
	//XClearWindow(m_display, m_window);
	if (m_szStatus != m_szOldStatus)
	{
		m_szOldStatus = m_szStatus;
	
		// Clear the whole area
		XClearArea(m_display, m_window, 0, 150, 280, 70, false);
	}
	else
	{
		XClearArea(m_display, m_window, 150, 150, 130, 70, false);
	}

	// Draw the status string anyway (incase the window was hidden and restored)
	XDrawString(m_display, m_window, m_gc, (36 + 50) - (stringWidth / 2), 186, m_szStatus.c_str(), m_szStatus.length());
	
	// Get draw the status
	XDrawString(m_display, m_window, m_gc, (153 + 50) - (statusWidth / 2), 186, progress, strlen(progress));

	// Flush the display
	XFlush(m_display);

	// Unlock the display for other threads
	XUnlockDisplay(m_display);
}

void XlibWindow::SetCurrent(float currentBytes)
{
	m_dCurrentBytes = currentBytes;
}

void XlibWindow::SetTotal(float totalBytes)
{
	m_dTotalBytes = totalBytes;
}

void XlibWindow::SetStatus(const char* status)
{
	m_szStatus = status;
}

void XlibWindow::SetPercentage(float percentage)
{
	if (percentage > 100)
	{
		m_percentage = 100;
	}
	else if (percentage < 0)
	{
		m_percentage = 0;
	}
	else
	{
		m_percentage = percentage;
	}
}

void XlibWindow::IncrementPercentage(float amount)
{
	ERROR_OUTPUT(__func__);
	if (m_percentage < 100)
		SetPercentage(m_percentage + amount);
}

void* XlibWindow::WrapThread(void* data)
{
	ERROR_OUTPUT(__func__);
    XlibWindow *ourThis = reinterpret_cast<XlibWindow *>(data);
    
    ourThis->RunThread();

	return NULL;
}

void XlibWindow::RunThread()
{
	ERROR_OUTPUT(__func__);
	while (m_run)
	{
		if (XPending(m_display) == 0)
		{
			usleep(10000);
			Redraw();
		}
		else
		{
			XNextEvent(m_display, &m_event);
			HandleEvent(m_event);
		}
	}
}

bool XlibWindow::IsRunning()
{
	return m_run;
}
