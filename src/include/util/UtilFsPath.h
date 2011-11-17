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

#ifndef DESURA_UTIL_FSPATH_H
#define DESURA_UTIL_FSPATH_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>

#ifdef WIN32
#include <xstring>
#endif

namespace UTIL
{
namespace FS
{
	class File
	{
	public:
		File();
		explicit File(const char* file);
		explicit File(std::string file);

		std::string getFile() const;
		std::string getFileExt() const;

		File& operator =(const File &rhs);

		bool operator==(const File &other) const;
		bool operator!=(const File &other) const;

	private:
		std::string m_szFile;
	};

	class Path
	{
	public:
		Path();
		Path(std::wstring path, std::wstring file, bool lastIsFolder);
		Path(std::string path, std::string file, bool lastIsFolder);
		Path(const Path& path);

		File getFile() const;

		std::string getLastFolder() const;
		std::string getFolderPath() const;
		std::string getFullPath() const;
		std::string getShortPath(size_t idealLen, bool withFile) const;

		std::string operator +(const std::string &rhs);
		std::string operator +=(const std::string &rhs);

		Path& operator +(const Path &rhs);
		Path& operator +=(const Path &rhs);
		Path& operator =(const Path &rhs);

		File& operator +=(const File &rhs);

		bool operator==(const Path &other) const;
		bool operator!=(const Path &other) const;

		static char GetDirSeperator();

		size_t getFolderCount();
		std::string getFolder(size_t index);

#ifdef NIX
		bool m_absolutePath;
#endif
	private:
		void parsePath(std::string path, bool lastIsFolder = false);

		std::vector<std::string> m_vPath;
		File m_File;
	};


	inline Path PathWithFile(const char* path)
	{
		if (!path)
			return Path("", "", false);

		return Path(path, "", true);
	}

	inline Path PathWithFile(const std::string& path)
	{
		return Path(path, "", true);
	}
}
}
#endif //DESURA_UTIL_FSPATH_H
