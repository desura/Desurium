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
#include "CefEvents.h"
#include "IPCBrowser.h"



EventHandler::EventHandler()
{
}

EventHandler::~EventHandler()
{
}

bool EventHandler::onNavigateUrl(const char* url, bool isMain)
{
	return GetIPCBrowser()->onNavigateUrl(url, isMain);
}

void EventHandler::onPageLoadStart()
{
	GetIPCBrowser()->onPageLoadStart();
}

void EventHandler::onPageLoadEnd()
{
	GetIPCBrowser()->onPageLoadEnd();
}

bool EventHandler::onJScriptAlert(const char* msg)
{
	return false;
}

bool EventHandler::onJScriptConfirm(const char* msg, bool* result)
{
	return false;
}

bool EventHandler::onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255])
{
	return false;
}

bool EventHandler::onKeyEvent(ChromiumDLL::KeyEventType type, int code, int modifiers, bool isSystemKey)
{
	return GetIPCBrowser()->onKeyEvent((int)type, code, modifiers, isSystemKey);
}

void EventHandler::onLogConsoleMsg(const char* message, const char* source, int line)
{
	GetIPCBrowser()->onLogConsoleMsg(message, source, line);
}

void EventHandler::launchLink(const char* url)
{
	GetIPCBrowser()->launchLink(url);
}

bool EventHandler::onLoadError(const char* errorMsg, const char* url, char* buff, size_t size)
{
	return false;
}

void EventHandler::HandleWndProc(int message, int wparam, int lparam)
{
}

bool EventHandler::HandlePopupMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo)
{
	size_t totSize = 31;

	std::vector<gcString> urls;

	urls.push_back(menuInfo->getLinkUrl());
	urls.push_back(menuInfo->getImageUrl());
	urls.push_back(menuInfo->getPageUrl());
	urls.push_back(menuInfo->getFrameUrl());
	urls.push_back(menuInfo->getSelectionText());
	urls.push_back(menuInfo->getMisSpelledWord());
	urls.push_back(menuInfo->getSecurityInfo());

	for (size_t x=0; x<urls.size(); x++)
		totSize += urls[x].size();

	std::vector<ChromiumDLL::ChromiumMenuItemI*> items;

	for (int x=0; x<menuInfo->getCustomCount(); x++)
		items.push_back(menuInfo->getCustomItem(x));

	for (size_t x=0; x<items.size(); x++)
		totSize += 1 + gcString(items[x]->getLabel()).size();

	char* pBuf = new char[totSize];

	int ti;
	LPCTSTR temp = pBuf;

	ti = menuInfo->getTypeFlags();
	CopyMemory((PVOID)temp, (const void*)&ti, 4);
	temp += 4;

	ti = menuInfo->getEditFlags();
	CopyMemory((PVOID)temp, (const void*)&ti, 4);
	temp += 4;

	int x, y;
	menuInfo->getMousePos(&x, &y);

	CopyMemory((PVOID)temp, (const void*)&x, 4);
	temp += 4;

	CopyMemory((PVOID)temp, (const void*)&y, 4);
	temp += 4;

	for (size_t x=0; x<urls.size(); x++)
	{
		unsigned char s = urls[x].size();

		CopyMemory((PVOID)temp, (const void*)&s, 1);
		temp += 1;

		CopyMemory((PVOID)temp, (const void*)urls[x].c_str(), s);
		temp += s;
	}

	ti = items.size();
	CopyMemory((PVOID)temp, (const void*)&ti, 4);
	temp += 4;

	for (size_t x=0; x<items.size(); x++)
	{
		ChromiumDLL::ChromiumMenuItemI* item = items[x];

		ti = item->getAction();
		CopyMemory((PVOID)temp, (const void*)&ti, 4);
		temp += 4;

		ti = item->getType();
		CopyMemory((PVOID)temp, (const void*)&ti, 4);
		temp += 4;

		gcString lable = item->getLabel();
		unsigned char s = lable.size();

		CopyMemory((PVOID)temp, (const void*)&s, 1);
		temp += 1;

		CopyMemory((PVOID)temp, (const void*)lable.c_str(), s);
		temp += s;

		s = item->isEnabled();
		CopyMemory((PVOID)temp, (const void*)&s, 1);
		temp += 1;

		s = item->isChecked();
		CopyMemory((PVOID)temp, (const void*)&s, 1);
		temp += 1;
	}

	ti = (int)menuInfo->getHWND();
	CopyMemory((PVOID)temp, (const void*)&ti, 4);
	temp += 4;

	bool res = GetIPCBrowser()->handlePopupMenu(IPC::PBlob(pBuf, totSize));

	safe_delete(pBuf);

	return res;
}
