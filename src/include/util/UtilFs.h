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

#ifndef DESURA_UTIL_FS_H
#define DESURA_UTIL_FS_H
#ifdef _WIN32
#pragma once
#endif

#include "util/UtilFsPath.h"
#include "util/UtilCallback.h"

namespace UTIL
{
namespace FS
{
#ifdef NIX
	//! Returns a string with words expanded (see http://linux.die.net/man/3/wordexp)
	//!
	//! @param file path to expand
	//! @return Converted string
	std::string expandPath(char* file);
	std::string expandPath(const char* file);
#endif
	//! Gets a file size
	//!
	//! @param file File path
	//! @return File size in bytes
	//!
	uint64 getFileSize(Path file);
	inline uint64 getFileSize(std::string file)
	{
		return getFileSize(UTIL::FS::PathWithFile(file));
	}

	//! Gets a folder size
	//!
	//! @param folder folder path
	//! @return folder size in bytes
	//!
	uint64 getFolderSize(Path folder);
	inline uint64 getFolderSize(std::string folder)
	{
		return getFolderSize(UTIL::FS::Path(folder, "", false));
	}

	//! Makes all folders along path
	//!
	//! @param path Path of folders to make
	//!
	void recMakeFolder(Path path);
	inline void recMakeFolder(std::string file)
	{
		recMakeFolder(UTIL::FS::Path(file, "", false));
	}

	//! Makes one folder
	//!
	//! @param path path to folder and folder to make (last part of path)
	//!
	void makeFolder(Path path);
	inline void makeFolder(std::string file)
	{
		makeFolder(UTIL::FS::PathWithFile(file));
	}

	//! Read all data from a file.
	//!
	//! @parma file Path to file
	//! @parma buff Buffer to save result in. Will allocate it. 
	//! @return Size of the buffer.
	//!
	uint32 readWholeFile(Path file, char** buf);
	inline uint32 readWholeFile(std::string file, char** buf)
	{
		return readWholeFile(UTIL::FS::PathWithFile(file), buf);
	}

	//! Copt a file from one place to another
	//!
	//! @param src File to copy
	//! @param dest File to copy to
	//!
	void copyFile(Path src, Path dest);
	inline void copyFile(std::string src, std::string dest)
	{
		copyFile(UTIL::FS::PathWithFile(src), UTIL::FS::PathWithFile(dest));
	}

	//! Copt a folder from one place to another
	//!
	//! @param src Folder to copy
	//! @param dest of folder
	//!
	void copyFolder(Path src, Path dest, std::vector<std::string> *vIgnoreList = NULL, bool copyOverExisting = true);
	inline void copyFolder(std::string src, std::string dest, std::vector<std::string> *vIgnoreList = NULL, bool copyOverExisting = true)
	{
		copyFolder(UTIL::FS::Path(src, "", false), UTIL::FS::Path(dest, "", false), vIgnoreList, copyOverExisting);
	}

	//! Moves a folder from one location to another
	//!
	//! @param src Folder to move
	//! @param dest Location to move to
	//!
	void moveFolder(Path src, Path dest);
	inline void moveFolder(std::string src, std::string dest)
	{
		moveFolder(UTIL::FS::Path(src, "", false), UTIL::FS::Path(dest, "", false));
	}


	//! Moves a file from one location to another
	//!
	//! @param src file to move
	//! @param dest Location to move to
	//!
	void moveFile(Path src, Path dest);
	inline void moveFile(std::string src, std::string dest)
	{
		moveFile(UTIL::FS::PathWithFile(src), UTIL::FS::PathWithFile(dest));
	}

	//! Removes a folder and all its contents
	//!
	//! @param src Folder to erase
	//!
	void eraseFolder(Path src);
	inline void eraseFolder(std::string src)
	{
		eraseFolder(UTIL::FS::PathWithFile(src));
	}

	//! Removes a file from the os
	//! 
	//! @param file Path to file
	//!
	void delFile(Path file);
	inline void delFile(std::string file)
	{
		delFile(UTIL::FS::PathWithFile(file));
	}

	//! Removes a folder and its contents from the os
	//!
	//! @param folder Path to folder
	//!
	void delFolder(Path folder);
	inline void delFolder(std::string folder)
	{
		delFolder(UTIL::FS::Path(folder, "", false));
	}

	//! Recurivly removes empty folders from the path
	//!
	//! @param path Path to start from
	//!
	void delEmptyFolders(Path path);
	inline void delEmptyFolders(std::string path)
	{
		delEmptyFolders(UTIL::FS::Path(path, "", false));
	}
	
	//! See if file is on drive and has size greater than zero
	//!
	//! @param file Path to file
	//! @return True if valid, false if not
	//!
	bool isValidFile(Path file);
	inline bool isValidFile(std::string file)
	{
		return isValidFile(UTIL::FS::PathWithFile(file));
	}

	//! See if folder is on drive and is a folder not a file
	//!
	//! @param folder Path to folder
	//! @return True if valid, false if not
	//!
	bool isValidFolder(Path folder);
	inline bool isValidFolder(std::string folder)
	{
		return isValidFolder(UTIL::FS::Path(folder, "", false));
	}

	//! See if a folder has any files in it.
	//! 
	//! @param path Path to folder
	//! @return True if empty, false if not
	//!
	bool isFolderEmpty(Path path);
	inline bool isFolderEmpty(std::string path)
	{
		return isFolderEmpty(UTIL::FS::Path(path, "", false));
	}

	time_t lastWriteTime(Path path);
	inline time_t lastWriteTime(std::string file)
	{
		return lastWriteTime(UTIL::FS::Path(file, "", true));
	}

	//! Gets all files in a folder
	//!
	//! @param path Folder path
	//! @param outList List of files found
	//! @param extsFilter Extension filter or null if no filter
	//!
	void getAllFiles(Path path, std::vector<Path> &outList, std::vector<std::string> *extsFilter);

	//! Gets all folders in a folder
	//!
	//! @param path Folder path
	//! @param outList List of folders found (raw folder name)
	//!
	void getAllFolders(Path path, std::vector<Path> &outList);

	//! Mode to open a file with
	enum FILE_MODE
	{
		FILE_READ,	//!< Open file for reading
		FILE_WRITE,	//!< Open file for writing
		FILE_APPEND,//!< Open file for appending
	};

	//! Reprents a raw file on the os that will auto close the handle once its deleted
	//!
	class FileHandle
	{
	public:
		//! Default constructor
		FileHandle();

		//! Alt constructor
		//! 
		//! @param fileName Name of file to open
		//! @param mode Mode to open a file with
		//! @param offset Default offset from start as file. Treats file as being (size - offset) big starting from offset
		//!
		FileHandle(const char* file, FILE_MODE mode, uint64 offset = 0);
		FileHandle(const Path path, FILE_MODE mode, uint64 offset = 0);

		//! 
		~FileHandle();

		FileHandle(const FileHandle& handle);
		FileHandle& operator=(const FileHandle& handle);

		//! Opens a new file and closes the old handle if still open
		//!
		//! @param fileName Name of file to open
		//! @param mode Mode to open a file with
		//! @param offset Default offset from start as file. Treats file as being (size - offset) big starting from offset
		//!
		void open(const char* file, FILE_MODE mode, uint64 offset = 0);
		void open(const Path path, FILE_MODE mode, uint64 offset = 0);

		//! Close currently opened file
		//!
		void close();

		//! Read data from a file
		//!
		//! @param buff Buffer to save to
		//! @param size Size to read
		//!
		void read(char* buff, uint32 size);

		void readCB(uint64 size, UTIL::CB::CallbackI* callback);

		template <typename T>
		void read(uint64 size, T &callback)
		{
			UTIL::CB::TemplateCallback<T> c(callback);
			readCB(size, &c);
		}

		template <typename T>
		void read(uint64 size, const T &callback)
		{
			UTIL::CB::TemplateCallback<const T> c(callback);
			readCB(size, &c);
		}

		//! Write data to a file
		//!
		//! @param buff Buffer to write
		//! @param size Ammount to write
		//!
		void write(const char* buff, uint32 size);

		//! Seeks to a spot in the file
		//!
		//! @param pos Position to seek to from start
		//!
		void seek(uint64 pos);

#ifdef WIN32
		//! Gets the native file system handle to the file
		//!
		//! @return Native handle
		//!
		HANDLE getHandle() const {return m_hFileHandle;}
#else
		//! Gets the native file system handle to the file
		//!
		//! @return Native handle
		//!
		FILE* getHandle() const {return m_hFileHandle;}
#endif		

		//! Is the current file handle open
		//!
		//! @return True if open, false if not
		//!
		bool isOpen() const {return m_bIsOpen;}

		//! Is the current file handle a valid file
		//!
		//! @return True if valid, false if not
		//!
		bool isValidFile() const {return (m_bIsOpen && m_hFileHandle);}

#ifdef NIX
		const char* getMode() const {return m_szMode.c_str();}
#endif

	private:
#ifdef WIN32
		HANDLE m_hFileHandle;
#else
		FILE* m_hFileHandle;
#endif

		bool m_bIsOpen;

#ifdef NIX
		std::string m_szMode;
#endif

#ifdef DEBUG
		std::string m_szFileName;
#endif

		uint64 m_uiOffset;
	};


	uint32 CRC32(const char* file);
}
}

#endif
