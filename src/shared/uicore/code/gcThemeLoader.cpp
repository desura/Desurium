/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
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
#include "gcThemeLoader.h"
#include "MainApp.h"

REGISTER_SCHEME(ThemeLoaderScheme);

ThemeLoaderScheme::ThemeLoaderScheme() : DesuraSchemeBase<ThemeLoaderScheme>("desura", "theme")
{
}

bool ThemeLoaderScheme::processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect)
{
	char wurl[255];
	request->getURL(wurl, 255);

	// desura://theme/ is 15 chars!

	gcString url = wurl;
	url = url.substr(15);
	url = UTIL::STRING::urlDecode(url);

	const char* themeFolder = GetGCThemeManager()->getThemeFolder();

	UTIL::FS::Path path(themeFolder, url, false);

	if (!UTIL::FS::isValidFile(path))
		path = UTIL::FS::Path(UTIL::OS::getDataPath(L"themes/default"),
			UTIL::STRING::toWStr(url), false);

	if (!UTIL::FS::isValidFile(path))
		return false;

#ifdef WIN32
	gcString localUrl("file:///{0}", path.getFullPath());
#else
	gcString localUrl("file://{0}", path.getFullPath());
#endif

	m_szRedirectUrl = UTIL::STRING::sanitizeFilePath(localUrl.c_str(), '/');
	*redirect = true;

	return true;
}

void ThemeLoaderScheme::cancel()
{
}

bool ThemeLoaderScheme::read(char* buffer, int size, int* readSize)
{
	return false;
}












RegisterSchemeHelper<StaticLoaderScheme> g_RS_Static;


StaticLoaderScheme::StaticLoaderScheme() : DesuraSchemeBase<StaticLoaderScheme>("desura", "static")
{
}

bool StaticLoaderScheme::processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect)
{
	char wurl[255];
	request->getURL(wurl, 255);

	// desura://static/ is 16 chars!

	gcString url = wurl;
	url = url.substr(16);
	url = UTIL::STRING::urlDecode(url);

	UTIL::FS::Path path(gcString("{0}/static", UTIL::OS::getDataPath()), url, false);

	if (!UTIL::FS::isValidFile(path))
		return false;

	gcString localUrl("file:///{0}", path.getFullPath());
	m_szRedirectUrl = UTIL::STRING::sanitizeFilePath(localUrl.c_str(), '/');

	*redirect = true;
	return true;
}

void StaticLoaderScheme::cancel()
{
}

bool StaticLoaderScheme::read(char* buffer, int size, int* readSize)
{
	return false;
}
