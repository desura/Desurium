/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "gcCheckBox.h"

#include "gcManagers.h"

#include "wx/brush.h"
#include "wx/dcclient.h"
#include "wx/dcscreen.h"
#include "wx/settings.h"

#include "wx/renderer.h"
#ifdef WIN32
	#include "wx/msw/dc.h"          // for wxDCTemp
	#include "wx/msw/uxtheme.h"
#else
	#include "wx/dc.h"
	#include "wx/button.h" // NOT SURE
#endif

gcCheckBox::gcCheckBox(wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style) : wxCheckBox(parent, id, label, pos, size, style)
{
	applyTheme();
}

void gcCheckBox::applyTheme()
{
	Managers::LoadTheme(this, "checkbox");
	SetBackgroundColour(GetParent()->GetBackgroundColour());
}

#ifdef WIN32
bool gcCheckBox::MSWOnDraw(WXDRAWITEMSTRUCT *item)
{
    DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)item;

    if ( !IsOwnerDrawn() || dis->CtlType != ODT_BUTTON )
        return wxCheckBoxBase::MSWOnDraw(item);

    // calculate the rectangles for the check mark itself and the label
    HDC hdc = dis->hDC;
    RECT& rect = dis->rcItem;
    RECT rectCheck,
         rectLabel;
    rectCheck.top =
    rectLabel.top = rect.top;
    rectCheck.bottom =
    rectLabel.bottom = rect.bottom;
    const int checkSize = GetBestSize().y;
    const int MARGIN = 3;

    const bool isRightAligned = HasFlag(wxALIGN_RIGHT);
    if ( isRightAligned )
    {
        rectCheck.right = rect.right;
        rectCheck.left = rectCheck.right - checkSize;

        rectLabel.right = rectCheck.left - MARGIN;
        rectLabel.left = rect.left;
    }
    else // normal, left-aligned checkbox
    {
        rectCheck.left = rect.left;
        rectCheck.right = rectCheck.left + checkSize;

        rectLabel.left = rectCheck.right + MARGIN;
        rectLabel.right = rect.right;
    }

    // show we draw a focus rect?
    const bool isFocused = IsChecked() || FindFocus() == this;


    // draw the checkbox itself
    wxDCTemp dc(hdc);

    int flags = 0;
    if ( !IsEnabled() )
        flags |= wxCONTROL_DISABLED;
    switch ( Get3StateValue() )
    {
        case wxCHK_CHECKED:
            flags |= wxCONTROL_CHECKED;
            break;

        case wxCHK_UNDETERMINED:
            flags |= wxCONTROL_PRESSED;
            break;

        default:
            wxFAIL_MSG( _T("unexpected Get3StateValue() return value") );
            // fall through

        case wxCHK_UNCHECKED:
            // no extra styles needed
            break;
    }

    if ( wxFindWindowAtPoint(wxGetMousePosition()) == this )
        flags |= wxCONTROL_CURRENT;

    wxRendererNative::Get().
        DrawCheckBox(this, dc, wxRectFromRECT(rectCheck), flags);

    // draw the text
    const wxString& label = GetLabel();

    // first we need to measure it
    UINT fmt = DT_NOCLIP;

    // drawing underlying doesn't look well with focus rect (and the native
    // control doesn't do it)
    if ( isFocused )
        fmt |= DT_HIDEPREFIX;
    if ( isRightAligned )
        fmt |= DT_RIGHT;
    // TODO: also use DT_HIDEPREFIX if the system is configured so

    // we need to get the label real size first if we have to draw a focus rect
    // around it
    if ( isFocused )
    {
        if ( !::DrawText(hdc, label.wx_str(), label.length(), &rectLabel,
                         fmt | DT_CALCRECT) )
        {
            wxLogLastError(_T("DrawText(DT_CALCRECT)"));
        }
    }

    if ( !IsEnabled() )
    {
        ::SetTextColor(hdc, ::GetSysColor(COLOR_GRAYTEXT));
    }

    if ( !::DrawText(hdc, label.wx_str(), label.length(), &rectLabel, fmt) )
    {
        wxLogLastError(_T("DrawText()"));
    }

	

    //// finally draw the focus
    if ( isFocused )
    {
		COLORREF colBg = wxColourToRGB(wxColor(GetGCThemeManager()->getColor("checkbox", "focus-fg")));
		HBRUSH hbrush = ::CreateSolidBrush(colBg);

        rectLabel.left--;
        rectLabel.right++;

        ::FrameRect(hdc, &rectLabel, hbrush);
		::DeleteObject(hbrush);
    }

    return true;
}
#endif
