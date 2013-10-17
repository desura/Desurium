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

#include "common.h"
#include <vector>
#include "TabHandler.h"
#include "TabPanel.h"

#include "boost\date_time.hpp"

TabHandler::TabHandler(TabPanelI* parent)
{
	m_pParent = parent;
	m_uiCurPanel = 0;

	m_tNextUpdate = boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time());
}

TabHandler::~TabHandler()
{

}


void TabHandler::clear()
{
	m_vPanelList.clear();
	m_uiCurPanel = 0;
}

void TabHandler::addPanel(TabPanelI* pan)
{
	if (pan)
	{
		m_vPanelList.push_back(pan);
		pan->setTabHandler(this);
	}
}


void TabHandler::onKeyDown(TabPanelI* pan, wxKeyEvent& event)
{
	boost::posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());
	if (now < m_tNextUpdate)
		return;

	m_tNextUpdate = now + boost::posix_time::milliseconds(100);

	for (size_t x=0; x<m_vPanelList.size(); x++)
	{
		if ( m_vPanelList[x] == pan)
		{
			m_uiCurPanel = x;
			break;
		}
	}


	switch (event.GetKeyCode())
	{
	case WXK_TAB:
		{
			if (event.m_shiftDown)
				prev();
			else
				next();

			break;
		}

	case WXK_LEFT:
		pan->onLeft();
		break;

	case WXK_RIGHT:
		pan->onRight();
		break;

	case WXK_UP:
		prev();
		break;

	case WXK_DOWN:
		next();
		break;
	};
}

void TabHandler::next()
{
	if (m_vPanelList.size()-1 == m_uiCurPanel)
	{
		m_vPanelList[m_uiCurPanel]->getTabHandler()->first();
	}
	else
	{
		m_uiCurPanel++;
		updatePanel();
	}
}

void TabHandler::prev()
{
	if (m_uiCurPanel == 0)
	{
		if (m_pParent->getTabHandler())
		{
			m_pParent->getTabHandler()->last();
		}
		else if (m_vPanelList.size() > 1)
		{
			m_uiCurPanel = m_vPanelList.size()-1;
			updatePanel();
		}
	}
	else
	{
		m_uiCurPanel--;
		updatePanel();
	}
}

void TabHandler::front(bool down)
{
	if (!down && m_pParent->getTabHandler())
	{
		m_pParent->getTabHandler()->front(false);
	}
	else
	{
		if (m_vPanelList[ 0 ]->getTabHandler())
			m_vPanelList[ 0 ]->getTabHandler()->front(true);
		else
			first();
	}
}

void TabHandler::end(bool down)
{
	if (!down && m_pParent->getTabHandler())
	{
		m_pParent->getTabHandler()->end(false);
	}
	else
	{
		if (m_vPanelList[ m_vPanelList.size()-1 ]->getTabHandler())
			m_vPanelList[ m_vPanelList.size()-1 ]->getTabHandler()->end(true);
		else
			last();
	}
}	

void TabHandler::first()
{
	m_uiCurPanel = 0;
	updatePanel();
}

void TabHandler::last()
{
	m_uiCurPanel = m_vPanelList.size()-1;
	updatePanel();
}

void TabHandler::updatePanel()
{
	if (m_uiCurPanel < m_vPanelList.size())
		m_vPanelList[m_uiCurPanel]->setFocus();
}
