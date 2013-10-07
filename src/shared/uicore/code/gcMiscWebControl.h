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

#ifndef DESURA_GCMISCWEBCONTROL_H
#define DESURA_GCMISCWEBCONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "gcWebControl.h"
#include "MainApp.h"
#include <wx/uri.h>


class gcURI : public wxURI
{
public:
	gcURI(gcString uri) : wxURI(uri.c_str())
	{
		m_query = "";
	}
};

template <typename T>
class gcMiscWebControlBase : public T
{
public:
	gcMiscWebControlBase(wxWindow* parent, const char* defaultUrl, const char* hostName) : T(parent, defaultUrl, hostName)
	{
		m_szLoadingPage = GetGCThemeManager()->getWebPage("loading");
		m_szCurrentUrl = defaultUrl;
		T::onNewURLEvent += delegate(this, &gcMiscWebControlBase::onNewUrl);
	}

	virtual void loadUrl(const gcString& url)
	{
		m_szCurrentUrl = gcString(url.c_str());
		T::loadUrl(url);
	}

protected:
	void onNewUrl(newURL_s& info)
	{
		gcString url(info.url);

		if (strncmp(url.c_str(), "desura://", 9) == 0)
		{
			info.stop = true;
			g_pMainApp->handleInternalLink(url.c_str());
		}

		//if we are loading a sub resource then let it
		if (!info.main)
			return;

		//Msg("On new url: %s\n", url.c_str());

		if (url == wxString("about:blank"))
			return;

		if (strncmp(url.c_str(), "javascript:", 11) == 0)
		{
			//dont do any thing
		}
		else if (strncmp(url.c_str(), "wyciwyg:", 8) == 0)
		{
			//dont do any thing
		}
		else if (isSafeUrl(url.c_str()))
		{
			if (!isCurrentUrl(url.c_str()) && url.find("/media/iframe") == std::string::npos)
			{
				info.stop = true;
				g_pMainApp->loadUrl(url.c_str(), COMMUNITY);
			}
		}
		else
		{
			info.stop = true;
			gcLaunchDefaultBrowser(info.url);
		}

		if (info.stop == false)
			m_szCurrentUrl = info.url;
	}

	bool isCurrentUrl(const char* url)
	{
		gcString str(url);

		//remove end slash
		if (str[str.size()-1] == '/')
			str[str.size()-1] = 0;

		gcURI u(str);
		gcURI c(m_szCurrentUrl);
		gcURI l(m_szLoadingPage);

		return u == c || c == l || u == l;
	}

private:
	gcString m_szLoadingPage;
	gcString m_szCurrentUrl;
};


typedef gcMiscWebControlBase<gcWebControl> gcMiscWebControl;

#ifdef WIN32
	#include "gcWebHost.h"
	typedef gcMiscWebControlBase<gcWebHost> gcMiscWebHost;
#else
	typedef gcMiscWebControlBase<gcWebControl> gcMiscWebHost;
#endif


#endif //DESURA_GCMISCWEBCONTROL_H
