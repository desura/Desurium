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
#include "util/UtilOs.h"

#include <boost/filesystem.hpp>

namespace UTIL
{
namespace OS
{

using namespace boost::filesystem;

std::wstring getDataPath(std::wstring extra)
{
#ifdef WIN32
	path dataDir = L".\\data";
#else
	path dataDir = L"@DATA_INSOURCE_DIR@";
#endif

	// we need this for windows, where all files are relative in the desura application folder or for local Linux
	// installations (if you want to move it around your Document directory)
	if (dataDir.is_relative())
		dataDir = getCurrentDir() / dataDir;

	return (dataDir / extra).wstring();
}

gcString getRuntimeLibPath()
{
	return getCurrentDir() + L"/";
}

}
}

#ifndef DESURA_OFFICIAL_BUILD
	std::string GetAppBuild()
	{
		return "@CPACK_PACKAGE_VERSION_PATCH@@CPACK_PACKAGE_VERSION_ADDITION@";
	}

	std::string GetAppBranch()
	{
		return "@CPACK_PACKAGE_VERSION_MAJOR@.@CPACK_PACKAGE_VERSION_MINOR@";
	}
#endif

