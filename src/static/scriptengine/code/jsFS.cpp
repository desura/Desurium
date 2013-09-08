/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
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
#include "jsFS.h"

void FromJSObject(UTIL::FS::FileHandle* &jsItem, JSObjHandle& arg)
{
	if (arg->isObject())
		jsItem = arg->getUserObject<UTIL::FS::FileHandle>();
	else
		jsItem = NULL;
}

FileSystemJSBinding::FileSystemJSBinding() : DesuraJSBase<FileSystemJSBinding>("fs", "installer_binding_fs.js")
{
	REG_SIMPLE_JS_FUNCTION( IsValidFile, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( IsValidFolder, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( CopyFile, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( DeleteFile, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( DeleteFolder, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( SetFolderPermissions, FileSystemJSBinding );
	REG_SIMPLE_JS_FUNCTION( GetFileSize, FileSystemJSBinding );

	REG_SIMPLE_JS_FUNCTION( OpenFileForWrite, FileSystemJSBinding );
	REG_SIMPLE_JS_OBJ_FUNCTION( WriteFile, FileSystemJSBinding );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( CloseFile, FileSystemJSBinding );
}

FileSystemJSBinding::~FileSystemJSBinding()
{
}

bool FileSystemJSBinding::IsValidFile(gcString file)
{
	return UTIL::FS::isValidFile(file);
}

bool FileSystemJSBinding::IsValidFolder(gcString folder)
{
	return UTIL::FS::isValidFolder(folder);
}

bool FileSystemJSBinding::CopyFile(gcString file, gcString dest)
{
	UTIL::FS::copyFile(file, dest);
	return UTIL::FS::isValidFile(dest);
}

bool FileSystemJSBinding::DeleteFile(gcString file)
{
	UTIL::FS::delFile(file);
	return UTIL::FS::isValidFile(file);
}

bool FileSystemJSBinding::SetFolderPermissions(gcString folder)
{
#ifdef WIN32
	try
	{
		UTIL::WIN::changeFolderPermissions(gcWString(folder));
	}
	catch (gcException&)
	{
		return false;
	}

	return true;
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

int32 FileSystemJSBinding::GetFileSize(gcString file)
{
	return (int32)UTIL::FS::getFileSize(file);
}

bool FileSystemJSBinding::DeleteFolder(gcString path)
{
	UTIL::FS::delFolder(path);
	return UTIL::FS::isValidFolder(path);
}

void* FileSystemJSBinding::OpenFileForWrite(gcString file)
{
	UTIL::FS::Path path = UTIL::FS::PathWithFile(file);
	UTIL::FS::recMakeFolder(path);

	UTIL::FS::FileHandle* handle = new UTIL::FS::FileHandle();

	try
	{
		handle->open(path, UTIL::FS::FILE_WRITE);
		return handle;
	}
	catch (gcException &e)
	{
		safe_delete(handle);
		Warning(gcString("Failed to to open file in scriptcore: {0}\n", e));
	}

	return NULL;
}

bool FileSystemJSBinding::WriteFile(UTIL::FS::FileHandle* handle, gcString string)
{
	if (!handle)
		return false;

	try
	{
		handle->write(string.c_str(), string.size());
		return true;
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to write to file in scriptcore: {0}\n", e));
	}

	return false;
}

void FileSystemJSBinding::CloseFile(UTIL::FS::FileHandle* handle)
{
	if (!handle)
		return;

	handle->close();
	safe_delete(handle);
}
