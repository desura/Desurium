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

#include <stdio.h>
#include "Common.h"

void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

template <typename F>
int copyFiles(std::vector<UTIL::FS::Path> &files, UTIL::FS::Path dest, F shouldConvertFn)
{
	for (size_t x=0; x<files.size(); x++)
	{
		for (size_t y=x+1; y<files.size(); y++)
		{
			if (files[x].getFile() == files[y].getFile())
			{
				printf("Cannot copy file %s as duplicate has been found. Please check that a filter file of same name doesnt exist.\n", files[x].getFile().getFile().c_str());
				return -4;
			}
		}
	}

	for (size_t x=0; x<files.size(); x++)
	{
		bool hasExt = false;
		std::string name = files[x].getFile().getFile();
		size_t pos = name.find_last_of(".");

		//remove ext
		if (pos != std::string::npos)
		{
			name = name.substr(0, pos);
			hasExt = true;
		}

		if (!shouldConvertFn(name))
			continue;

		if (hasExt)
			name += "." + files[x].getFile().getFileExt();

		dest += UTIL::FS::File(name);
		UTIL::FS::copyFile(files[x], dest);

		if (!UTIL::FS::isValidFile(dest))
		{
			printf("Failed to copy file: %s\n", dest.getFullPath().c_str());
			return -2;
		}
	}

	return 0;
}

template <typename F>
size_t filterFolders(std::vector<UTIL::FS::Path> &folders, std::vector<UTIL::FS::Path> &destFolders, UTIL::FS::Path destPath, F shouldConvertFn)
{
	std::vector<size_t> delVector;

	for (size_t x=0; x<folders.size(); x++)
	{
		std::string lastFolder = folders[x].getLastFolder();

		if (lastFolder == ".svn" || !shouldConvertFn(lastFolder))
		{
			delVector.push_back(x);
			continue;
		}

		UTIL::FS::Path destDup = destPath;
		destDup += lastFolder;

		destFolders.push_back(destDup);
	}

	std::for_each(delVector.rbegin(), delVector.rend(), [&folders](size_t index){
		folders.erase(folders.begin()+index);
	});

	return destFolders.size();
}

template <typename F>
int processFolder(UTIL::FS::Path src, UTIL::FS::Path dest, F shouldConvertFn)
{
	std::vector<UTIL::FS::Path> files;
	std::vector<UTIL::FS::Path> folders;
	std::vector<UTIL::FS::Path> destfolders;

	UTIL::FS::getAllFiles(src, files, NULL);
	UTIL::FS::getAllFolders(src, folders);

	UTIL::FS::recMakeFolder(dest);

	int res = copyFiles(files, dest, shouldConvertFn);

	if (res != 0)
		return res;

	size_t size = filterFolders(folders, destfolders, dest, shouldConvertFn);

	for (size_t x=0; x<size; x++)
	{
		for (size_t y=x+1; y<size; y++)
		{
			if (destfolders[x].getLastFolder() == destfolders[y].getLastFolder())
			{
				printf("Cannot copy folder %s as duplicate has been found. Please check that a filter folder of same name doesnt exist.\n", destfolders[x].getLastFolder().c_str());
				return -3;
			}
		}
	}

	for (size_t x=0; x<size; x++)
	{
		res = processFolder(folders[x], destfolders[x], shouldConvertFn);

		if (res != 0)
			return res;
	}

	return 0;
}

int shouldConvert(const char* convert, const char* ignore, std::string &name)
{
	size_t cSize = strlen(convert);
	size_t iSize = strlen(ignore);

	if (iSize < name.size())
	{
		std::string sub = name.substr(name.size()-iSize);

		if (sub == ignore)
			return false;
	}

	if (cSize < name.size())
	{
		std::string sub = name.substr(name.size()-cSize);

		if (sub == convert)
			name = name.substr(0, name.size()-cSize);
	}

	return true;
}

int main(int argc, char ** argv)
{
	if (argc < 5)
	{
		printf("gen_data_folder [org folder] [dest folder] [convert] [ignore]\n");
		return 0;
	}

	UTIL::FS::recMakeFolder(argv[1]);
	
	const char* convert = argv[3];
	const char* ignore = argv[4];

	auto shouldConvertFn = [convert, ignore](std::string &name) -> int 
	{
		return shouldConvert(convert, ignore, name);
	};

	return processFolder(UTIL::FS::Path(argv[1], "", false), UTIL::FS::Path(argv[2], "", false), shouldConvertFn);
}