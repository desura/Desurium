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
#include "gcTipWindow.h"

#include "wx/dcclient.h"
#include "wx/timer.h"
#include "wx/settings.h"

#include "gcManagers.h"

// karolherbst: where is this class uesd?
// it is needed to work with wxPopupWindowHandler or will be wxEvtHandler enough?
//class WXDLLIMPEXP_FWD_CORE wxPopupWindowHandler;
class WXDLLIMPEXP_CORE wxPopupWindowHandler : public wxEvtHandler
{
public:
    wxPopupWindowHandler(wxPopupTransientWindow *popup);

protected:
    // event handlers
    void OnLeftDown(wxMouseEvent& event);

private:
    wxPopupTransientWindow *m_popup;

    DECLARE_EVENT_TABLE()
    wxDECLARE_NO_COPY_CLASS(wxPopupWindowHandler);
};


static const wxCoord TEXT_MARGIN_X = 3;
static const wxCoord TEXT_MARGIN_Y = 3;


class gcTipWindowView : public wxWindow
{
public:
    gcTipWindowView(wxWindow *parent);

    // event handlers
    void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);

    // calculate the client rect we need to display the text
    void Adjust(const wxString& text, wxCoord maxLength);

private:
    gcTipWindow* m_parent;
};


bool SetWindowTransparent(wxWindow* win, wxByte alpha)
{
#ifdef WIN32
    LONG exstyle = GetWindowLong((HWND)win->GetHWND(), GWL_EXSTYLE);

    // if setting alpha to fully opaque then turn off the layered style
    if (alpha == 255)
    {
        SetWindowLong((HWND)win->GetHWND(), GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);
        win->Refresh();
        return true;
    }

    // Otherwise, set the layered style if needed and set the alpha value
    if ((exstyle & WS_EX_LAYERED) == 0 )
        SetWindowLong((HWND)win->GetHWND(), GWL_EXSTYLE, exstyle | WS_EX_LAYERED);

    if ( SetLayeredWindowAttributes((HWND)win->GetHWND(), 0, (BYTE)alpha, LWA_ALPHA) )
        return true;
#endif
    return false;
}

// ----------------------------------------------------------------------------
// gcTipWindow
// ----------------------------------------------------------------------------

gcTipWindow::gcTipWindow(wxWindow *parent, const wxString& text, wxCoord maxLength, gcTipWindow** windowPtr, wxRect *rectBounds) : wxPopupTransientWindow(parent)
{
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &gcTipWindow::onMouseCaptureLost, this);

    SetTipWindowPtr(windowPtr);
    if ( rectBounds )
    {
        SetBoundingRect(*rectBounds);
    }

	Managers::LoadTheme(this, "toolTip");

    // set size, position and show it
    m_view = new gcTipWindowView(this);
    m_view->Adjust(text, maxLength);
    m_view->SetFocus();

    int x, y;
    wxGetMousePosition(&x, &y);

    // we want to show the tip below the mouse, not over it
    //
    // NB: the reason we use "/ 2" here is that we don't know where the current
    //     cursors hot spot is... it would be nice if we could find this out
    //     though
    y += wxSystemSettings::GetMetric(wxSYS_CURSOR_Y) / 2;

    Position(wxPoint(x, y), wxSize(0,0));
    Popup(m_view);

	Color bg = GetThemeManager()->getColor("toolTip", "bg");
	SetWindowTransparent(this, bg.alpha);
}

gcTipWindow::~gcTipWindow()
{
    if ( m_windowPtr )
        *m_windowPtr = NULL;
}

void gcTipWindow::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (HasCapture())
		ReleaseMouse();

	Close();
}

void gcTipWindow::OnIdle(wxIdleEvent& event)
{
	event.Skip();
}

bool gcTipWindow::Show(bool show)
{
	return wxPopupWindow::Show(show);
}

void gcTipWindow::SetTipWindowPtr(gcTipWindow** windowPtr) 
{ 
	m_windowPtr = windowPtr; 
}

void gcTipWindow::OnDismiss()
{
    Close();
}

void gcTipWindow::SetBoundingRect(const wxRect& rectBound)
{
    m_rectBound = rectBound;
}

void gcTipWindow::Close()
{
    if ( m_windowPtr )
    {
        *m_windowPtr = NULL;
        m_windowPtr = NULL;
    }

    Show(false);
    Destroy();
}

void gcTipWindow::Popup(wxWindow *winFocus)
{
    const wxWindowList& children = GetChildren();

    if ( children.GetCount() )
        m_child = children.GetFirst()->GetData();
    else
        m_child = this;

    Show();

    if (!m_handlerPopup)
        m_handlerPopup = new wxPopupWindowHandler(this);

    m_child->PushEventHandler(m_handlerPopup);
}

// ----------------------------------------------------------------------------
// gcTipWindowView
// ----------------------------------------------------------------------------

gcTipWindowView::gcTipWindowView(wxWindow *parent) : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	Bind(wxEVT_RIGHT_DOWN, &gcTipWindowView::OnMouseClick, this);
	Bind(wxEVT_LEFT_DOWN, &gcTipWindowView::OnMouseClick, this);
	Bind(wxEVT_MIDDLE_DOWN, &gcTipWindowView::OnMouseClick, this);

	Bind(wxEVT_PAINT, &gcTipWindowView::OnPaint, this);

	Managers::LoadTheme(this, "toolTip");

    m_parent = (gcTipWindow*)parent;
}

void gcTipWindowView::Adjust(const wxString& text, wxCoord maxLength)
{
    wxClientDC dc(this);
    dc.SetFont(GetFont());

    // calculate the length: we want each line be no longer than maxLength
    // pixels and we only break lines at words boundary
    wxString current;
    wxCoord height, width,
            widthMax = 0;
    m_parent->m_heightLine = 0;

    bool breakLine = false;
    for ( const wxChar *p = text.c_str(); ; p++ )
    {
        if ( *p == _T('\n') || *p == _T('\0') )
        {
            dc.GetTextExtent(current, &width, &height);
            if ( width > widthMax )
                widthMax = width;

            if ( height > m_parent->m_heightLine )
                m_parent->m_heightLine = height;

            m_parent->m_textLines.Add(current);

            if ( !*p )
            {
                // end of text
                break;
            }

            current.clear();
            breakLine = false;
        }
        else if ( breakLine && (*p == _T(' ') || *p == _T('\t')) )
        {
            // word boundary - break the line here
            m_parent->m_textLines.Add(current);
            current.clear();
            breakLine = false;
        }
        else // line goes on
        {
            current += *p;
            dc.GetTextExtent(current, &width, &height);
            if ( width > maxLength )
                breakLine = true;

            if ( width > widthMax )
                widthMax = width;

            if ( height > m_parent->m_heightLine )
                m_parent->m_heightLine = height;
        }
    }

    // take into account the border size and the margins
    width  = 2*(TEXT_MARGIN_X + 1) + widthMax;
    height = 2*(TEXT_MARGIN_Y + 1) + wx_truncate_cast(wxCoord, m_parent->m_textLines.GetCount())*m_parent->m_heightLine;
    m_parent->SetClientSize(width, height);
    SetSize(0, 0, width, height);
}

void gcTipWindowView::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    wxRect rect;
    wxSize size = GetClientSize();
    rect.width = size.x;
    rect.height = size.y;

    // first filll the background
    dc.SetBrush(wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(GetForegroundColour(), 1, wxPENSTYLE_SOLID));
    dc.DrawRectangle(rect);

    // and then draw the text line by line
    dc.SetTextBackground(GetBackgroundColour());
    dc.SetTextForeground(GetForegroundColour());
    dc.SetFont(GetFont());

    wxPoint pt;
    pt.x = TEXT_MARGIN_X;
    pt.y = TEXT_MARGIN_Y;
    size_t count = m_parent->m_textLines.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        dc.DrawText(m_parent->m_textLines[n], pt);
        pt.y += m_parent->m_heightLine;
    }
}

void gcTipWindowView::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    m_parent->Close();
}
