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

#include "Common.h"
#include <wx/wx.h>

#include "gcCustomMenu.h"
#include "gcThemeManager.h"

class MenuColors
{
public:
	MenuColors()
	{
		BGColor = wxColor(GetGCThemeManager()->getColor("menu", "bg"));
		BGFocusColor = wxColor(GetGCThemeManager()->getColor("menu", "focus-bg"));
		FGColor = wxColor(GetGCThemeManager()->getColor("menu", "fg"));
		FGFocusColor = wxColor(GetGCThemeManager()->getColor("menu", "focus-fg"));
		BorderColor = wxColor(GetGCThemeManager()->getColor("menu", "border"));
		SpacerColor = wxColor(GetGCThemeManager()->getColor("menu", "spacer"));
		FGColorDisabled = wxColor(GetGCThemeManager()->getColor("menu", "na-fg"));

		CheckNormal = GetGCThemeManager()->getImageHandle("#menu_checked");
		CheckFocus = GetGCThemeManager()->getImageHandle("#menu_checked_focus");
	}

	wxColor BGColor;
	wxColor FGColor;
	wxColor FGFocusColor;
	wxColor BGFocusColor;
	wxColor FGColorDisabled;
	wxColor BorderColor;
	wxColor SpacerColor;

	gcImageHandle CheckNormal;
	gcImageHandle CheckFocus;
};

MenuColors* g_MColors = NULL;

class AutoDeleteMC
{
public:
	~AutoDeleteMC()
	{
		safe_delete(g_MColors);
	}
};


AutoDeleteMC admc;


gcMenuItem::gcMenuItem(wxMenu *parentMenu, int id, const wxString& name, const wxString& help, wxItemKind kind, wxMenu *subMenu) : wxMenuItem(parentMenu, id, name, help, kind, subMenu)
{
#ifdef WIN32
	SetOwnerDrawn();	
#endif

	if (!g_MColors)
		g_MColors = new MenuColors();

#ifdef WIN32
	//need to set this here so we will be self drawn
	SetTextColour(wxColor(0,0,0));
#endif

	m_bExtendedItems = false;
}

#ifdef WIN32
bool gcMenuItem::OnMeasureItem(size_t *pwidth, size_t *pheight)
{
	wxScreenDC dc;
	wxRect textRect = dc.GetTextExtent(this->GetItemLabel());

	size_t w = 0;
	size_t h = 0;

	if (IsSeparator())
	{
		h = 9;
		w = 1;
	}
	else
	{
		h = 23;
		w = textRect.GetWidth();
	}

	if (w < 76)
		w = 76;

	if (m_bExtendedItems)
		w += 25;

	*pwidth = w;
	*pheight = h;

	return true;
}

bool gcMenuItem::OnDrawItem(wxDC& dc, const wxRect& rc, wxOwnerDrawn::wxODAction act, wxOwnerDrawn::wxODStatus stat)
{
	bool isSelected = ((stat & ODS_SELECTED) && (act & (ODA_SELECT | ODA_DRAWENTIRE)));
	bool isGrayed = ((stat & ODS_GRAYED) && (act & (ODA_SELECT | ODA_DRAWENTIRE)));

	if (isSelected && !isGrayed && !IsSeparator())
	{
		dc.SetTextForeground(g_MColors->FGFocusColor);
		dc.SetBrush(wxBrush(g_MColors->BGFocusColor));
		dc.SetPen(wxPen(g_MColors->BGFocusColor));
	}
	else
	{
		if (isGrayed)
		{
			dc.SetTextForeground(g_MColors->FGColorDisabled);
		}
		else
		{
			dc.SetTextForeground(g_MColors->FGColor);
		}

		dc.SetBrush(wxBrush(g_MColors->BGColor));
		dc.SetPen(wxPen(g_MColors->BGColor));
	}

	dc.DrawRectangle(rc);

	if (IsSeparator())
	{
		dc.SetBrush(wxBrush(g_MColors->SpacerColor));
		dc.SetPen(wxPen(g_MColors->SpacerColor));

		int x1 = rc.GetLeft() + 2;
		int x2 = rc.GetRight() - 2;
		int y = rc.GetTop() + rc.GetHeight()/2;

		dc.DrawLine(x1, y, x2, y);
	}
	else
	{
		wxRect textExtent = dc.GetTextExtent(GetItemLabel());

		int x= rc.GetLeft() + 15;

		if (m_bExtendedItems)
			x += 10;

		int y= rc.GetTop() + (rc.GetHeight()-textExtent.GetHeight())/2;

		dc.DrawText(GetItemLabel(), x, y);
	}

	wxBitmap icon;

	if (IsChecked())
	{
		if (isSelected)
			icon = wxBitmap(g_MColors->CheckFocus.getImg()->Scale(16,16));
		else
			icon = wxBitmap(g_MColors->CheckNormal.getImg()->Scale(16,16));
	}

	if (!IsCheckable() && GetBitmap().IsOk())
	{
		icon = GetBitmap();
	}

	if (icon.IsOk())
	{
		wxSize bsize = icon.GetSize();

		int x = rc.GetLeft() + (25-bsize.GetWidth())/2;
		int y = rc.GetTop() + (rc.GetHeight()-bsize.GetHeight())/2;

		dc.DrawBitmap(icon, wxPoint(x, y), true);
	}

	return true;
}
#endif


#ifdef NIX
#include <gtk/gtk.h>
extern "C"
{
	static void deactivate_menu(GtkWidget*, gcMenu* menu)
	{
		safe_delete(menu);
	}
}
#endif

gcMenu::gcMenu(bool autoDel)
{
#ifdef NIX
	if (autoDel)
		g_signal_connect(m_menu, "selection-done", G_CALLBACK(deactivate_menu), this);
#endif
}

void gcMenu::layoutMenu()
{

	bool extended = false;

	wxMenuItemList list = GetMenuItems();

	for (size_t x=0; x<list.size(); x++)
	{
		wxMenuItem *item = list[x];
		gcMenu* subMenu = dynamic_cast<gcMenu*>(list[x]->GetSubMenu());

		if (item->IsCheckable() || item->GetBitmap().IsOk())
		{
			extended = true;
		}

		if (subMenu)
			subMenu->layoutMenu();
	}

	if (extended)
	{
		for (size_t x=0; x<list.size(); x++)
		{
			gcMenuItem *item = dynamic_cast<gcMenuItem*>(list[x]);
				
			if (item)
				item->setExtended();
		}
	}

#ifdef WIN32
	HMENU handle = (HMENU)this->GetHMenu();

	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_BACKGROUND;
	mi.hbrBack = CreateSolidBrush(RGB(g_MColors->BorderColor.Red(), g_MColors->BorderColor.Green(), g_MColors->BorderColor.Blue()));

	SetMenuInfo(handle, &mi);
#endif
}
