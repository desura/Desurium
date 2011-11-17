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
#include "util/UtilFsPath.h"


namespace UTIL
{
namespace FS
{


File::File()
{
}

File::File(const char* file)
{
	if (file)
		m_szFile = std::string(file);
}

File::File(std::string file)
{
	m_szFile = file;
}

std::string File::getFile() const
{
	return m_szFile;
}

std::string File::getFileExt() const
{
	size_t pos = m_szFile.find_last_of('.');

	if (pos == std::string::npos)
		return "";

	return m_szFile.substr(pos+1); 
}

File& File::operator =(const File &rhs)
{
	m_szFile = rhs.getFile();
	return *this;
}

bool File::operator==(const File &other) const
{
	return m_szFile == other.getFile();
}

bool File::operator!=(const File &other) const
{
	return !(*this == other);
}

Path::Path()
{
#ifdef NIX
	m_absolutePath = false;
#endif
}

Path::Path(std::wstring path, std::wstring file, bool lastIsFolder)
{
#ifdef NIX
	m_absolutePath = (path.size() > 0 && path[0] == L'/');
#endif

	parsePath(gcString(path), lastIsFolder);

	if (lastIsFolder == false)
		m_File = File(gcString(file));
}

Path::Path(std::string path, std::string file, bool lastIsFolder)
{
#ifdef NIX
	m_absolutePath = (path.size() > 0 && path[0] == '/');
#endif

	parsePath(path, lastIsFolder);

	if (lastIsFolder == false)
		m_File = File(file);
}

Path::Path(const Path& path)
{
#ifdef NIX
	m_absolutePath = (path.getFullPath().size() > 0 && path.getFullPath()[0] == '/');
#endif

	m_File = path.getFile();
	m_vPath = path.m_vPath;
}

File Path::getFile() const
{
	return m_File;
}

std::string Path::getLastFolder() const
{
	if (m_vPath.size() == 0)
		return "";

	return m_vPath.back();
}

std::string Path::getFolderPath() const
{

	std::string out;

#ifdef NIX
	if (m_absolutePath && m_vPath.size() > 0)
		out += GetDirSeperator();
#endif

	for (size_t x=0; x<m_vPath.size(); x++)
	{
		out += m_vPath[x];
		
		if (x+1 < m_vPath.size())
			out += GetDirSeperator();
	}

	return out;
}

std::string Path::getFullPath() const
{
	std::string out = getFolderPath();
	std::string file = m_File.getFile();

	if (out.size() > 0 && file.size() > 0)
		out += GetDirSeperator();

	out += file;

	return out;
}

char Path::GetDirSeperator()
{
#ifdef WIN32
	return '\\';
#else
	return '/';
#endif
}

void Path::parsePath(std::string p, bool lastIsFolder)
{

	std::vector<std::string> vList;

	size_t last;
	size_t pos;

#ifdef WIN32
	char good = '\\';
	char bad = '/';
#else
	char good = '/';
	char bad = '\\';
#endif

	pos = 0;

	do
	{
		pos = p.find(bad, pos);

		if (pos != std::string::npos)
		  p[pos] = good;
	}
	while (pos != std::string::npos);


	pos = 0;
	last = 0;

	do
	{
		pos = p.find(good, last);

		if (pos != std::string::npos)
		{
			m_vPath.push_back(p.substr(last, pos-last));
		}
		else
		{
			m_vPath.push_back(p.substr(last));
		}

		last = pos+1;
	}
	while (pos != std::string::npos);

	std::vector<size_t> delList;

	for (size_t x=0; x<m_vPath.size(); x++)
	{
		if (m_vPath[x].size() == 0)
			delList.push_back(x);
	}

	for (size_t x=delList.size(); x>0; x--)
	{
		m_vPath.erase(m_vPath.begin()+delList[x-1]);
	}

	if (lastIsFolder && m_vPath.size() > 0)
	{
		m_File = File(m_vPath.back().c_str());
		m_vPath.pop_back();
	}
}

std::string Path::operator +(const std::string &rhs)
{
	Path path(*this);
	path += rhs;

	return path.getFullPath();
}

std::string Path::operator +=(const std::string &rhs)
{
	parsePath(rhs);
	return getFullPath();
}

Path& Path::operator +=(const Path &rhs)
{
	for (size_t x=0; x<rhs.m_vPath.size(); x++)
		m_vPath.push_back(rhs.m_vPath[x]);

	return *this;
}

Path& Path::operator =(const Path &rhs)
{
#ifdef NIX
	m_absolutePath = rhs.m_absolutePath;
#endif
	m_vPath = rhs.m_vPath;
	m_File = rhs.m_File;

	return *this;
}

File& Path::operator +=(const File &rhs)
{
	m_File = rhs;
	return m_File;
}

bool Path::operator==(const Path &other) const
{
	if (other.m_vPath.size() != m_vPath.size())
		return false;

	for (size_t x=0; x<other.m_vPath.size(); x++)
	{
		if (other.m_vPath[x] != m_vPath[x])
			return false;
	}

	return m_File == other.m_File;
}

bool Path::operator!=(const Path &other) const 
{
	return !(*this == other);
}


std::string Path::getShortPath(size_t idealLen, bool withFile) const
{
	std::string fullPath;
	
	if (withFile)
		fullPath = getFullPath();
	else
		fullPath = getFolderPath();

	size_t len = fullPath.size();

	if (idealLen > (uint32)len)
		return fullPath;

	std::vector<std::string> list = m_vPath;

	if (withFile && m_File.getFile() != "")
		list.push_back(m_File.getFile());

	if (list.size() <= 1)
		return fullPath;

	size_t start = 0;
	size_t stop = list.size()-1;
	size_t count = 0;

	while (count < idealLen)
	{
		if (start == stop)
			break;

		size_t startLen = list[start].size();
		size_t endLen = list[stop].size();

		if (count + startLen < idealLen)
		{
			start++;
			count += startLen;
		}

		if (start == stop)
			break;

		if (count + endLen < idealLen)
		{
			stop--;
			count += endLen;
		}
		else
		{
			//failed to add start and end strings, lets stop
			break;
		}
	}

	Path out;
	bool first = true;

	for (size_t x=0; x<list.size(); x++)
	{
		if (x<start || x>stop)
		{
			out += list[x];
		}
		else if (first)
		{
			first = false;
			out += "...";
		}
	}

	return out.getFullPath();
}

size_t Path::getFolderCount()
{
	return m_vPath.size();
}

std::string Path::getFolder(size_t index)
{
	if (index >= m_vPath.size())
		return "";

	return m_vPath[index];
}

}
}
