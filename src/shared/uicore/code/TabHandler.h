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

#ifndef DESURA_TABHANDLER_H
#define DESURA_TABHANDLER_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include "wxControls/gcControls.h"
#include "boost/date_time.hpp"

class TabPanelI;

class TabHandler
{
public:
	TabHandler(TabPanelI* parent);
	~TabHandler();

	void clear();
	void addPanel(TabPanelI* pan);

	void onKeyDown(TabPanelI* pan, wxKeyEvent& event);

	void next();
	void prev();

	void front(bool down = false);
	void end(bool down = false);

	void first();
	void last();

protected:
	void updatePanel();

private:
	boost::posix_time::ptime m_tNextUpdate;


	std::vector<TabPanelI*> m_vPanelList;
	unsigned int m_uiCurPanel;

	TabPanelI* m_pParent;
};

#endif //DESURA_TABHANDLER_H
