///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : MenuInfo.cpp
//   Description :
//      [TODO: Write the purpose of MenuInfo.cpp.]
//
//   Created On: 9/18/2010 10:01:52 AM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "MenuInfo.h"

ChromiumMenuItem::ChromiumMenuItem(CefMenuHandler::MenuItem item)
{
	m_MenuItem = item;
}

int ChromiumMenuItem::getAction()
{
	return m_MenuItem.action;
}

int ChromiumMenuItem::getType()
{
	return m_MenuItem.type;
}

const char* ChromiumMenuItem::getLabel()
{
	return m_MenuItem.label.str;
}

bool ChromiumMenuItem::isEnabled()
{
	return m_MenuItem.enabled?true:false;
}

bool ChromiumMenuItem::isChecked()
{
	return m_MenuItem.checked?true:false;
}




ChromiumMenuInfo::ChromiumMenuInfo(CefMenuHandler::MenuInfo info, MenuHandle_t hwnd)
{
	m_MenuInfo = info;
	m_Hwnd = hwnd;

	for (int x=0; x<info.customSize; x++)
	{
		m_vMenuItems.push_back( ChromiumMenuItem(info.customItems[x]) );
	}
}

ChromiumDLL::ChromiumMenuInfoI::TypeFlags ChromiumMenuInfo::getTypeFlags()
{
	return (ChromiumDLL::ChromiumMenuInfoI::TypeFlags)m_MenuInfo.typeFlags;
}

ChromiumDLL::ChromiumMenuInfoI::EditFlags ChromiumMenuInfo::getEditFlags()
{
	return (ChromiumDLL::ChromiumMenuInfoI::EditFlags)m_MenuInfo.editFlags;
}

void ChromiumMenuInfo::getMousePos(int* x, int* y)
{
	*x = m_MenuInfo.x;
	*y = m_MenuInfo.y;
}

const char* ChromiumMenuInfo::getLinkUrl()
{
	return (const char*)m_MenuInfo.linkUrl.str;
}

const char* ChromiumMenuInfo::getImageUrl()
{
	return (const char*)m_MenuInfo.imageUrl.str;
}

const char* ChromiumMenuInfo::getPageUrl()
{
	return (const char*)m_MenuInfo.pageUrl.str;
}

const char* ChromiumMenuInfo::getFrameUrl()
{
	return (const char*)m_MenuInfo.frameUrl.str;
}

const char* ChromiumMenuInfo::getSelectionText()
{
	return (const char*)m_MenuInfo.selectionText.str;
}

const char* ChromiumMenuInfo::getMisSpelledWord()
{
	return (const char*)m_MenuInfo.misspelledWord.str;
}

const char* ChromiumMenuInfo::getSecurityInfo()
{
	return (const char*)m_MenuInfo.securityInfo.str;
}

int* ChromiumMenuInfo::getHWND()
{
	return (int*)m_Hwnd;
}

int ChromiumMenuInfo::getCustomCount()
{
	return m_MenuInfo.customSize;
}

ChromiumDLL::ChromiumMenuItemI* ChromiumMenuInfo::getCustomItem(size_t index)
{
	if (index >= m_vMenuItems.size())
		return NULL;

	return &m_vMenuItems[index];
}
