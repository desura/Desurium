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


#ifndef DESURA_JSFS_H
#define DESURA_JSFS_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"

class FileSystemJSBinding : public DesuraJSBase<FileSystemJSBinding>
{
public:
	FileSystemJSBinding();
	~FileSystemJSBinding();

protected:
	bool IsValidFile(gcString file);
	bool IsValidFolder(gcString folder);

	bool CopyFile(gcString file, gcString dest);
	bool DeleteFile(gcString file);

	bool DeleteFolder(gcString path);

	bool SetFolderPermissions(gcString folder);

	int32 GetFileSize(gcString file);

	void* OpenFileForWrite(gcString file);
	bool WriteFile(UTIL::FS::FileHandle* handle, gcString string);
	void CloseFile(UTIL::FS::FileHandle* handle);
};



#endif //DESURA_JSFS_H
