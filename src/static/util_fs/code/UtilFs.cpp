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


//make sure to BOOST_FILESYSTEM_VERSION=2

#include "Common.h"
#include "util/UtilFs.h"
#include "boost/filesystem.hpp"

#include <string>

namespace bf = boost::filesystem;

namespace UTIL
{
namespace MISC
{
	unsigned long CRC32(const unsigned char byte, unsigned long dwCrc32);
}

namespace FS
{

//file handling class
FileHandle::FileHandle()
{
	m_hFileHandle = NULL;
	m_bIsOpen = false;
}

FileHandle::FileHandle(const char* fileName, FILE_MODE mode, uint64 offset)
{
	m_hFileHandle = NULL;
	m_bIsOpen = false;
	open(fileName, mode, offset);
}

FileHandle::FileHandle(const Path path, FILE_MODE mode, uint64 offset)
{
	m_hFileHandle = NULL;
	m_bIsOpen = false;
	open(path.getFullPath().c_str(), mode, offset);
}

FileHandle::~FileHandle()
{
	if (m_bIsOpen)
		close();
}

void FileHandle::open(const Path path, FILE_MODE mode, uint64 offset)
{
	open(path.getFullPath().c_str(), mode, offset);
}

void FileHandle::readCB(uint64 size, UTIL::CB::CallbackI* callback)
{
	unsigned char buff[10*1024];
	uint32 buffSize = 10*1024;

	while (size > 0)
	{
		if (buffSize > size)
			buffSize = (uint32)size;

		read((char*)buff, buffSize);
		
		if (callback->onData(buff, buffSize))
			break;

		size -= buffSize;
	};
}

//////////////////////////


void printError( bf::wfilesystem_error e);

uint64 getFileSize(Path szfile)
{
	try
	{
		boost::uintmax_t fs = bf::file_size(bf::wpath(gcWString(szfile.getFullPath())));
		return (uint64)fs;
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
	return 0;
}

uint64 getFolderSize(Path folder)
{
	uint64 ret = 0;

	try
	{
		bf::wpath path(gcWString(folder.getFolderPath()));

		bf::wrecursive_directory_iterator endIter; 
		bf::wrecursive_directory_iterator dirIter( path );  

		while (endIter != dirIter)
		{
			if (bf::is_directory(*dirIter) == false)
				ret += (uint64)bf::file_size(*dirIter);

			++dirIter;
		}
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}

	return ret;
}

void makeFolder(Path name)
{
	try
	{
		bf::create_directory( bf::wpath(gcWString(name.getFolderPath())) );
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
}

void recMakeFolder(Path name)
{
	try
	{
		bf::create_directories( bf::wpath(gcWString(name.getFolderPath())) );
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}

}

void moveFolder(Path src, Path dest)
{
	if (!isValidFolder(src))
		return;

	recMakeFolder(dest);

	try
	{
		bf::rename(bf::wpath(gcWString(src.getFolderPath())), bf::wpath(gcWString(dest.getFolderPath())) );
	}
	catch (bf::wfilesystem_error e)
	{
	}
}

void moveFile(Path src, Path dest)
{
	if (!isValidFile(src))
		return;

	recMakeFolder(dest);

	try
	{
		bf::rename(bf::wpath(gcWString(src.getFullPath())), bf::wpath(gcWString(dest.getFullPath())));
	}
	catch (bf::wfilesystem_error e)
	{
	}
}

void eraseFolder(Path src)
{
	try
	{
		bf::remove_all( bf::wpath(gcWString(src.getFolderPath())) );
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
}


bool isValidFile(Path file)
{
	try
	{
		return (bf::exists( bf::wpath(gcWString(file.getFullPath())) ) && !bf::is_directory( bf::wpath(gcWString(file.getFullPath())) ));
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
	return false;
}


bool isValidFolder(Path folder)
{
	try
	{
		return bf::is_directory( bf::wpath(gcWString(folder.getFolderPath())) );
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
	return false;
}

void delFile(Path file)
{
	if (isValidFile(file))
	{
		try
		{
			bf::remove(bf::wpath(gcWString(file.getFullPath())));
		}
		catch (bf::wfilesystem_error e)
		{
			printError(e);
		}
	}
}

void delFolder(Path filePath)
{
	if (isValidFolder(filePath))
	{
		try
		{
			bf::remove_all(bf::wpath(gcWString(filePath.getFolderPath())));
		}
		catch (bf::wfilesystem_error e)
		{
			printError(e);
		}
	}
}

bool isFolderEmpty(Path filePath)
{
	if (isValidFolder(filePath))
	{
		return bf::is_empty(bf::wpath(gcWString(filePath.getFolderPath())));
	}

	return true;
}


void delEmptyFolders(Path filePath)
{
	if (!isValidFolder(filePath))
		return;

	std::vector<UTIL::FS::Path> folders;
	UTIL::FS::getAllFolders(filePath, folders);

	for (size_t x=0; x<folders.size(); x++)
	{
		
#ifdef NIX
		if (bf::is_symlink(bf::wpath(gcWString(filePath.getFolderPath()))))
			continue;
#endif

		delEmptyFolders(folders[x]);
	}

	if (isFolderEmpty(filePath))
		delFolder(filePath);
};

void printError( bf::wfilesystem_error e)
{
#ifndef NIX
	const char* what = e.what();
	//has a problem under linx about path1

#ifdef printf
	#define RE_DEF_PRINTF
	#undef printf
#endif

	if (strcmp("not_found_error", what) == 0)
	{
		printf("WARN: File %ls doe.\n", e.path1().string().c_str());
	}
	else if (strcmp("path_error", what) == 0)
	{
		printf("WARN: Bad Path %ls.\n", e.path1().string().c_str());
	}

#ifdef RE_DEF_PRINTF
	#define printf PrintfMsg
#endif

#endif
}


uint32 readWholeFile(Path path, char** buffer)
{
	uint64 size = UTIL::FS::getFileSize(path);

	if (size>>32)
		throw gcException(ERR_INVALID, gcString("File {0} is bigger than 4gb. Cant read whole file.", path.getFullPath()));

	uint32 size32 = (uint32)size;

	safe_delete(*buffer);
	*buffer = new char[size32+1];

	UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_READ);
	fh.read(*buffer, size32);

	(*buffer)[size32] = 0;

	return size32;
}

void copyFile(Path src, Path dest)
{
	try
	{
		bf::remove(bf::wpath(gcWString(dest.getFullPath())));
		bf::copy_file(bf::wpath(gcWString(src.getFullPath())), bf::wpath(gcWString(dest.getFullPath())));
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
}

void copyFolder(Path src, Path dest, std::vector<std::string> *vIgnoreList, bool copyOverExisting)
{
	UTIL::FS::recMakeFolder(dest);
	bf::wdirectory_iterator end_itr;

	try
	{
		for (bf::wdirectory_iterator itr(gcWString(src.getFolderPath())); itr!=end_itr; ++itr)
		{
			bf::wpath path(itr->path());

			std::string szPath = gcString(path.directory_string().c_str());
			std::string szNode = gcString(path.leaf());

			bool found = false;
			bool isDir = bf::is_directory(itr->status());

			if (vIgnoreList)
			{
				for (size_t x=0; x<(*vIgnoreList).size(); x++)
				{
					if ((*vIgnoreList)[x] == szNode)
					{
						found = true;
						break;
					}
				}
			}

			if (found)
				continue;

			Path tdest = dest;

			if (isDir)
			{
				tdest += szNode;
				UTIL::FS::copyFolder(Path(szPath, "", false), tdest, vIgnoreList);
			}
			else if (copyOverExisting == false || UTIL::FS::isValidFile(tdest) == false)
			{
				tdest += UTIL::FS::File(szNode);
				UTIL::FS::copyFile(Path(szPath, "", true), tdest);
			}
		}
	}
	catch (bf::wfilesystem_error e)
	{
		printError(e);
	}
}

time_t lastWriteTime(Path path)
{
	return boost::filesystem::last_write_time(bf::wpath(gcWString(path.getFullPath())));
}

void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter)
{
	bf::wpath full_path(gcWString(path.getFolderPath()), bf::native);

	if (!isValidFolder(path))
		return;

	bf::wdirectory_iterator end_iter;
	for (bf::wdirectory_iterator dirIt(full_path); dirIt!=end_iter; ++dirIt)
	{
		if (!bf::is_directory(dirIt->status()))
		{
			std::wstring filePath(dirIt->path().leaf().c_str());

			Path subPath(path);
			subPath += File(gcString(filePath));

			if (!extsFilter)
			{
				outList.push_back(subPath);
			}
			else
			{
				std::vector<std::string> extList = *extsFilter;

				for (size_t x=0; x<extList.size(); x++)
				{
					if (Safe::stricmp(extList[x].c_str(), subPath.getFile().getFileExt().c_str()) ==0)
						outList.push_back(subPath);
				}
			}
		}
	}
}

void getAllFolders(Path path, std::vector<Path> &outList)
{
	bf::wpath full_path(gcWString(path.getFolderPath()), bf::native);

	if (!isValidFolder(path))
		return;

	bf::wdirectory_iterator end_iter;
	for (bf::wdirectory_iterator dirIt(full_path); dirIt!=end_iter; ++dirIt)
	{
		if (bf::is_directory(dirIt->status()))
		{
			Path subPath(path);

			gcWString wpath(dirIt->path().leaf().c_str());
			subPath += Path(gcString(wpath), "", false);

			outList.push_back(subPath);
		}
	}
}



uint32 CRC32(const char* file)
{
	if (!file)
		return -1;


	unsigned long ulCRC = 0xFFFFFFFF; //Initilaize the CRC.

	uint64 size = UTIL::FS::getFileSize(file);
	uint64 done = 0;

	uint32 buffSize = 10 * 1024;
	char buff[10 * 1024];

	try
	{
		UTIL::FS::FileHandle fh(file, UTIL::FS::FILE_READ);

		while (done < size)
		{
			if (buffSize > (size-done))
				buffSize = (uint32)(size-done);

			fh.read(buff, buffSize);

			for (size_t x=0; x<buffSize; x++)
				ulCRC = UTIL::MISC::CRC32(buff[x], ulCRC);

			done += buffSize;
		}
	}
	catch (...)
	{
		return -1;
	}

	return ~ulCRC; //Finalize the CRC and return.;
}

}
}
