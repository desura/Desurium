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

#ifndef DESURA_DOWNLOADPROVIDER_H
#define DESURA_DOWNLOADPROVIDER_H
#ifdef _WIN32
#pragma once
#endif

#include "XMLMacros.h"

namespace MCFCore
{
namespace Misc
{

//! DownloadProvider class stores infomation about servers that are avaliable to download MCF content from
//! 
class DownloadProvider
{
public:
	enum PROVIDER
	{
		ADD,	//!< Using new provider (i.e. add to banner)
		REMOVE,	//!< Stop using provider (i.e. remove from banner)
	};

	//! Default Constructor
	DownloadProvider()
	{
	}

	//! Copy Constructor
	DownloadProvider(MCFCore::Misc::DownloadProvider* prov)
	{
		if (!prov)
			return;

		m_szName = gcString(prov->getName());
		m_szUrl = gcString(prov->getUrl());
		m_szBanner = gcString(prov->getBanner());
		m_szProvUrl = gcString(prov->getProvUrl());
	}

	//! Alt Constructor
	//!
	//! @param n Provider Name
	//! @param u Download Url
	//! @param b Banner Url
	//! @param p Provider Url
	//!
	DownloadProvider(const char* n, const char* u, const char* b, const char* p)
	{
		m_szName = gcString(n);
		m_szUrl = gcString(u);
		m_szBanner = gcString(b);
		m_szProvUrl = gcString(p);
	}

	//! Alt Constructor from xml
	//! 
	//! @param node Xml node to get info from
	//!
	DownloadProvider(TiXmlElement* node)
	{
		XML::GetChild("banner", m_szBanner, node);
		XML::GetChild("provider", m_szName, node);
		XML::GetChild("provlink", m_szProvUrl, node);
		XML::GetChild("link", m_szUrl, node);
	}

	//! Checks to see if its a valid banner
	//! 
	//! @return True if valid, False if not
	//!
	bool isValid()
	{
		return (m_szName.size() > 0 && m_szUrl.size() > 0 && m_szBanner.size() > 0 && m_szProvUrl.size() > 0);
	}

	//! Gets the provider name
	//!
	//! @return Provider name
	//!
	const char* getName()
	{
		return m_szName.c_str();
	}

	//! Gets the download url
	//!
	//! @return Download url
	//!
	const char* getUrl()
	{
		return m_szUrl.c_str();
	}

	//! Gets the banner url
	//!
	//! @return Banner Url
	//!
	const char* getBanner()
	{
		return m_szBanner.c_str();
	}

	//! Gets the providers url
	//!
	//! @return Provider url
	//!
	const char* getProvUrl()
	{
		return m_szProvUrl.c_str();
	}


	//! Sets the banner url
	//!
	//! @param banner New banner Url
	//!
	void setBanner(const char* banner)
	{
		m_szBanner = gcString(banner);
	}

private:
	gcString m_szName;
	gcString m_szUrl;
	gcString m_szBanner;
	gcString m_szProvUrl;
};

}
}

#endif //DESURA_DOWNLOADPROVIDER_H
